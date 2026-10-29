/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 
 * holds architecture specific functions for x86 and x86_64 architecture needed for the
 * static executable rewriter
 */

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <set>
#include <map>
#include <sstream>

#include "emitElfStatic.h"
#include "Symtab.h"
#include "Symbol.h"
#include "Archive.h"
#include "Object.h"
#include "Region.h"
#include "debug.h"
#include "common/src/arch-power.h"
#include "elf.h"
#include "unaligned_memory_access.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace NS_power;

static const unsigned PPC32_WIDTH = 4;
static const unsigned PPC64_WIDTH = 8;

static const Offset GOT_RESERVED_SLOTS = 0;

unsigned int setBits(unsigned int target, unsigned int pos, unsigned int len, unsigned int value) {
  rewrite_printf("setBits target 0x%lx value 0x%lx pos %u len %u \n", (unsigned long)target, (unsigned long)value, pos, len);
// There are three parts of the target - 0:pos, pos:len, len:32 
// We want to create a mask with 0:pos and len:32 set to 1
    unsigned int mask;
    mask = ((1 << len) - 1) << pos;
    rewrite_printf(" mask 0x%lx value 0x%lx \n", (unsigned long)mask, (unsigned long)value);
    value = value & mask;
    target = target | value;
    rewrite_printf( "setBits target 0x%lx value 0x%lx pos %u len %u \n", (unsigned long)target, (unsigned long)value, pos, len);

    return target;
}

// The size of the return type of 'setBits' is used to determine the number of bytes
// written to specific ELF fields. It's imperative they match the sizes of the types
// provided by elfutils.
static_assert(sizeof(decltype(setBits(0,0,0,0))) == sizeof(Elf32_Word), "setBits return type is incompatible with Elf32_Word");
static_assert(sizeof(decltype(setBits(0,0,0,0))) == sizeof(Elf64_Word), "setBits return type is incompatible with Elf64_Word");


#if defined(os_freebsd)
#define R_X86_64_JUMP_SLOT R_X86_64_JMP_SLOT
#endif

// Used in an assert so needs to be a macro
#define UNKNOWN_ADDRESS_WIDTH_ASSERT "An unknown address width was encountered, can't continue"

/* NOTE:
 * As most of these functions are defined per architecture, the description of
 * each of these functions is in the emitElfStatic header. Comments describing
 * the function interface are explicitly left out.
 */

/** 
 *
 * Given a relocation, determines if the relocation corresponds to a .ctors or .dtors
 * table that requires special consideration. Modifies the passed symbol offset to
 * point to the right table, if applicable.
 *
 * rel          The relocation entry to examine
 * globalOffset The offset of the linked code (used for symbol offset calculation)
 * lmap         Holds information about .ctors/.dtors tables
 * errMsg       Set on error
 * symbolOffset Modified by this routine to contain the offset of the table
 *
 * Returns true, if there are no errors including the case where the relocation 
 * entry doesn't reference the .ctors/.dtors tables.
 */

static bool computeCtorDtorAddress(relocationEntry &rel, Offset globalOffset,
        LinkMap &lmap, string &errMsg, Offset &symbolOffset)
{
    if( rel.name() ==  SYMTAB_CTOR_LIST_REL ) {
        // This needs to be: (the location of the .ctors table)
        if( lmap.newCtorRegions.size() > 0 ) {
            symbolOffset = lmap.ctorRegionOffset + globalOffset;
	    rewrite_printf("new CTOR computeCtorDtorAddress symbolOffset 0x%lx \n", symbolOffset);
        }else if( lmap.originalCtorRegion != NULL ) {
            symbolOffset = lmap.originalCtorRegion->getMemOffset();
	    rewrite_printf("original CTOR computeCtorDtorAddress symbolOffset 0x%lx \n", symbolOffset);
        }else{
            errMsg = "Failed to locate original .ctors Region -- cannot apply relocation";
            rewrite_printf("Failed to locate original .ctors Region -- cannot apply relocation\n");
            return false;
        }
    }else if( rel.name() == SYMTAB_DTOR_LIST_REL ) {
        // This needs to be: (the location of the .dtors table)
        if( lmap.newDtorRegions.size() > 0 ) {
            symbolOffset = lmap.dtorRegionOffset + globalOffset;
        }else if( lmap.originalDtorRegion != NULL ) {
            symbolOffset = lmap.originalDtorRegion->getMemOffset();
        }else{
            errMsg = "Failed to locate original .dtors Region -- cannot apply relocation";
            rewrite_printf("Failed to locate original .dtors Region -- cannot apply relocation\n");
            return false;
        }
    }
    return true;
}


bool emitElfStatic::archSpecificRelocation(Symtab* targetSymtab, Symtab* srcSymtab,
					   char *targetData, relocationEntry &rel,
					   Offset dest, Offset relOffset, Offset globalOffset, LinkMap &lmap,
					   string &errMsg) 
{
	Symbol *dynsym = rel.getDynSym();
	
	Offset newTOCoffset = 0;

	// This is an added file, thus there's only one TOC value, so look up @0. 
	Offset curTOCoffset = srcSymtab->getTOCoffset((Offset) 0);

	// Usually, there is only one TOC.
	// TODO: if multiple TOCs are needed
	newTOCoffset = curTOCoffset;

	rewrite_printf(" archSpecificRelocation %s\ndynsym %s, reloc offset 0x%lx\n new TOC 0x%lx, cur TOC 0x%lx\n dest 0x%lx \n", 
		       rel.name().c_str(), 
		       dynsym->getMangledName().c_str(), relOffset,
		       newTOCoffset, curTOCoffset, dest);

       if (rel.getRelType() == R_PPC64_JMP_SLOT) {
          // This is a 24-byte structure with the following form:
          // bytes 0..8: pointer to a function
          // bytes 9..15: TOC
          // bytes 16..23: unused (by us)
          // Instead of hard-coding it, we treat it as a pair of relocations:
          // R_PPC64_GLOB_DAT (for the pointer)
          // R_PPC64_TOC (for the TOC)
          relocationEntry func = rel;
          func.setRelType(R_PPC64_GLOB_DAT);
          if (!archSpecificRelocation(targetSymtab, srcSymtab, targetData, func,
                                      dest, relOffset, globalOffset, lmap, errMsg)) {
             return false;
          }
          relocationEntry toc = rel;
          toc.setRelType(R_PPC64_TOC);
          // Also shift its target address by 8 so it overwrites the second slot
          toc.setTargetAddr(toc.target_addr() + 8);
          if (!archSpecificRelocation(targetSymtab, srcSymtab, targetData, toc,
                                      dest + 8, relOffset + 8, globalOffset, lmap, errMsg)) {
             return false;
          }
          return true;
       }

	int relocation_length = sizeof(Elf64_Word)*8; // in bits
	int relocation_pos = 0; // in bits
        /*
         * Referring to the SYSV 386 supplement:
         *
         * All relocations on x86 are one word32 == Elf32_Word
         *
         * S = symbolOffset
         * A = addend
         * P = relOffset
         */
       
        Offset symbolOffset = rel.getDynSym()->getOffset();

        Elf64_Word addend = 0;
        if( rel.regionType() == Region::RT_REL ) {
            memcpy(&addend, &targetData[dest], sizeof(Elf64_Word));
        }else if( rel.regionType() == Region::RT_RELA ) {
            addend = rel.addend();
        }

	// Handle TOC-changing inter-module calls
	// handleInterModule returns false if it's not its problem. 
	if (rel.getRelType() == R_PPC64_REL24 &&
	    newTOCoffset != curTOCoffset && 
	    handleInterModuleSpecialCase(targetSymtab, srcSymtab, lmap,
					 targetData, rel,
					 newTOCoffset, curTOCoffset, dest, 
					 relOffset, globalOffset)) {
	  return true;
	}

	// If symbol is .toc, we must return the got offset
	if (rel.name() == ".toc") {
	  vector<Region *> allRegions;
	  srcSymtab->getAllRegions(allRegions);
	  vector<Region *>::iterator region_it;
	  for(region_it = allRegions.begin(); region_it != allRegions.end(); ++region_it) {
	    if((*region_it)->getRegionName().compare(".toc") == 0) {
	      map<Region *, LinkMap::AllocPair>::iterator result = lmap.regionAllocs.find(*region_it);
	      if( result != lmap.regionAllocs.end() ) {
		Offset regionOffset = result->second.second;
		symbolOffset = globalOffset + regionOffset;
		rewrite_printf("\tregionOffset 0x%lx symbolOffset 0x%lx \n", regionOffset, symbolOffset);
		break;
	      }
	    }
	  }
    	}

	if (rel.name() == ".TOC.")
	  symbolOffset = newTOCoffset;

        rewrite_printf("\trelocation for '%s': TYPE = %s(%lu) S = %lx A = %x P = %lx Total 0x%lx \n",
                rel.name().c_str(), 
                relocationEntry::relType2Str(rel.getRelType(), addressWidth_),
                rel.getRelType(), symbolOffset, addend, relOffset, symbolOffset+addend);


        int64_t relocation = 0;
        map<Symbol *, Offset>::iterator result;
        stringstream tmp;

        switch(rel.getRelType()) {
	  /* PowerPC64 relocations defined by the ABIs */
	case R_PPC64_ADDR64:
	case R_PPC64_GLOB_DAT:	
	  relocation_length = 64;
	  relocation = symbolOffset + addend;
	  break;
	case R_PPC64_REL24:
	case R_PPC64_GOT_TPREL16_DS:
	  relocation_length = 24;
	  // Skip the last bit, which is 1 representing setting link register
	  // Skip the second-last bit, which represents relative branch
	  relocation_pos = 2;
	  //relocation = (symbolOffset + addend - relOffset)>> 2;
	  //Relocation R_PPC64_REL24 means R2 is already set up by the caller.
	  //So, we skip the first two instructions to the local entry
	  relocation = symbolOffset + addend - relOffset + 8;
	  break;
	case R_PPC64_GOT_TPREL16_HA:
	  relocation_length = 16;
	  relocation = symbolOffset + addend ; //- relOffset;
	  relocation = (relocation + 0x8000) >> 16;
	  break;
	case R_PPC64_GOT_TPREL16_LO_DS:
	  relocation_length = 16;
	  relocation = symbolOffset + addend ;// - relOffset;
	  relocation = relocation & 0xffff;
	  break;
	case R_PPC64_REL32:
	  relocation_length = 24;
	  relocation_pos = 6;
	  relocation = symbolOffset + addend - relOffset;
	  break;
	case R_PPC64_REL64:
	  relocation = symbolOffset + addend - relOffset;
	  break;
	case R_PPC64_TLS:
	  rewrite_printf("TLS - UNIMPLEMENTED\n");
	  break;
	case R_PPC64_TOC16:
	  relocation_length = 16;
	  relocation = symbolOffset + addend - newTOCoffset;
	  break;
	case R_PPC64_TOC16_HA:
	  relocation_length = 16;
	  relocation = symbolOffset + addend - newTOCoffset;
	  relocation = (relocation + 0x8000) >> 16;
	  break;
	case R_PPC64_TOC16_LO:
	  relocation_length = 16;
	  relocation = symbolOffset + addend - newTOCoffset;
	  relocation = relocation & 0xffff;
	  break;

	case R_PPC64_TOC16_DS:
	  relocation_length = 16;
	  relocation = (symbolOffset + addend - newTOCoffset) >> 2 ;
	  relocation = relocation << 2;
	  break;
	case R_PPC64_TOC16_LO_DS:
	  relocation_length = 16;
	  relocation = symbolOffset + addend - newTOCoffset;
	  relocation = relocation & 0xffff;
	  break;

	case R_PPC64_TOC:
	  // I don't know how we never had to deal with these before...
	  relocation_length = 64;
	  relocation = newTOCoffset;
	  break;

	case R_PPC64_REL16_HA:
	  relocation_length = 16;
	  relocation = (symbolOffset + addend - relOffset);
	  relocation = (relocation + 0x8000) >> 16; 
	  break;
	case R_PPC64_REL16_LO:
	  relocation_length = 16;
	  relocation = (symbolOffset + addend - relOffset) & 0xffff;
	  break;
	case R_PPC64_TPREL16_HA:
	  relocation_length = 16;
	  relocation = (symbolOffset - 0x7000 + 0x8000) >> 16;
	  break;
	case R_PPC64_TPREL16_LO:
	  relocation_length = 16;
	  relocation = (symbolOffset - 0x7000) & 0xffff;
	  break;
	default:
	  fprintf(stderr, "Unhandled relocation type %lu\n",rel.getRelType());
	  assert(0);
	}
        rewrite_printf("\tbefore: relocation = 0x%lx @ 0x%lx target data 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx \n",
                       static_cast<unsigned long>(relocation),
                       static_cast<unsigned long>(relOffset),
                       static_cast<unsigned long>(targetData[dest-2]),
                       static_cast<unsigned long>(targetData[dest-1]),
                       static_cast<unsigned long>(targetData[dest]),
                       static_cast<unsigned long>(targetData[dest+1]),
                       static_cast<unsigned long>(targetData[dest+2]),
                       static_cast<unsigned long>(targetData[dest+3]));

	if (relocation_length == 64) {
		char *td = (targetData + dest - (dest%8));
		auto const target = static_cast<uint64_t>(relocation);
        	memcpy(td, &target, 2*sizeof(Elf64_Word));
	} else {
		char *td = (targetData + dest - (dest%4));
	        auto target = Dyninst::read_memory_as<Elf64_Word>(td);
	        target = setBits(target, relocation_pos, relocation_length, relocation);
        	memcpy(td, &target, sizeof(Elf64_Word));
	}

        rewrite_printf("\tafter: relocation = 0x%lx @ 0x%lx target data 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx \n",
                       static_cast<unsigned long>(relocation),
                       static_cast<unsigned long>(relOffset),
                       static_cast<unsigned long>(targetData[dest-2]),
                       static_cast<unsigned long>(targetData[dest-1]),
                       static_cast<unsigned long>(targetData[dest]),
                       static_cast<unsigned long>(targetData[dest+1]),
                       static_cast<unsigned long>(targetData[dest+2]),
                       static_cast<unsigned long>(targetData[dest+3]));
    return true;
}

bool emitElfStatic::checkSpecialCaseSymbols(Symtab *, Symbol *) {
    return true;
}

/* The TLS implementation on ppc is Variant 1 */

Offset emitElfStatic::layoutTLSImage(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap) {
    return tlsLayoutVariant1(globalOffset, dataTLS, bssTLS, lmap);
}

Offset emitElfStatic::adjustTLSOffset(Offset curOffset, Offset /*tlsSize*/) {
    return curOffset;
}

char emitElfStatic::getPaddingValue(Region::RegionType /*rtype*/) {
   // Can't noop-pad because this returns a char, not an unsigned
   return 0x0;
}

void emitElfStatic::cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs,
                                            Region *dataTLS, Region *bssTLS) 
{
    tlsCleanupVariant2(regionAllocs, dataTLS, bssTLS);
}

Offset emitElfStatic::getGOTSize(Symtab * /*target*/, LinkMap &lmap, Offset & /*layoutStart*/) {
    Offset size = 0;

    unsigned slotSize = sizeof(Elf64_Addr);

    // According to the ELF abi, entries 0, 1, 2 are reserved in a GOT on x86
    if( lmap.gotSymbolTable.size() > 0 ) {
       rewrite_printf("Determining new GOT size: %lu symtab entries, %lu reserved slots, %u slot size\n",
                      lmap.gotSymbolTable.size(), GOT_RESERVED_SLOTS, slotSize);
        size = (lmap.gotSymbolTable.size()+GOT_RESERVED_SLOTS)*slotSize;
    }

    return size;
}

Offset emitElfStatic::getGOTAlign(LinkMap &) {

    if( PPC32_WIDTH == addressWidth_ ) {
	return 0;
    }
    if( PPC32_WIDTH == addressWidth_ ) {
        return sizeof(Elf32_Word);
    }else if( PPC64_WIDTH == addressWidth_ ) {
        return sizeof(Elf64_Xword);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return 0;
}

void emitElfStatic::buildGOT(Symtab * /*target*/, LinkMap &lmap) {

    if( PPC32_WIDTH == addressWidth_ ) {
	return;
    }
    char *targetData = lmap.allocatedData;

    unsigned slotSize = 0;
    if( PPC32_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf32_Addr);
    }else if( PPC64_WIDTH == addressWidth_ ) {
        slotSize = sizeof(Elf64_Addr);
    }else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    Offset curOffset = 0;

    // For each GOT symbol, allocate an entry and copy the value of the
    // symbol into the table, additionally store the offset in the GOT
    // back into the map
    memset(&targetData[lmap.gotRegionOffset], 0, GOT_RESERVED_SLOTS*slotSize);
    curOffset += GOT_RESERVED_SLOTS*slotSize;

    rewrite_printf("Copied GOT_RESERVED_SLOTS %lu, curOffset 0x%lx\n",
		   GOT_RESERVED_SLOTS, curOffset);

    vector<pair<Symbol *, Offset> >::iterator sym_it;
    rewrite_printf("Copying in %lu symbol table entries\n", lmap.gotSymbolTable.size());

    for(sym_it = lmap.gotSymbolTable.begin(); sym_it != lmap.gotSymbolTable.end(); ++sym_it) {
        Offset value = sym_it->first->getOffset()+sym_it->second;
        memcpy(&targetData[lmap.gotRegionOffset + curOffset], &value, slotSize);

        sym_it->second = curOffset;
	rewrite_printf(" Building GOT at 0x%lx is 0x%lx name %s Offset 0x%lx addend 0x%lx addr 0x%lx \n", 
		       curOffset, value,  
		       sym_it->first->getPrettyName().c_str(),  sym_it->first->getOffset(), 
		       sym_it->second, lmap.gotRegionOffset + curOffset);
        curOffset += slotSize;
    }
    rewrite_printf("curOffset is 0x%lx, calculated GOT size is 0x%lx\n", curOffset, lmap.gotSize);
    assert(curOffset == lmap.gotSize);
}

/*
 * .ctors and .dtors section handling
 * 
 * .ctors/.dtors sections are not defined by the ELF standard, LSB defines them.
 * This is why this implementation is specific to Linux and x86.
 *
 * Layout of .ctors and .dtors sections on Linux x86
 *
 * Executable .ctors/.dtors format (size in bytes = n)
 *
 *  byte 0..3    byte 4..7     byte 8..11        byte n-4..n-1
 * 0xffffffff <func. ptr 1> <func. ptr 2> ...  0x00000000
 *
 * Relocatable file .ctors/.dtors format (size in bytes = n)
 *
 *   byte 0..3         byte n-4..n-1
 * <func. ptr 1> ... <last func. ptr>
 *
 * The layout is the same on Linux x86_64 except each entry is 8 bytes
 * instead of 4. So the header and trailler are the same, but extended to
 * 8 bytes.
 */
static const string DTOR_NAME(".fini_array");
static const string CTOR_NAME(".init_array");
static const string TOC_NAME(".toc");

Offset emitElfStatic::layoutNewCtorRegion(LinkMap &lmap) {
    /* 
     * .ctors sections are processed in reverse order on Linux x86. New .ctors
     * sections need to be placed before the original .ctors section
     */

    Offset retOffset = lmap.ctorRegionOffset;
    retOffset += addressWidth_;

    pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;

    set<Region *, CtorComp<Region*> >::iterator reg_it;
    for(reg_it = lmap.newCtorRegions.begin(); reg_it != lmap.newCtorRegions.end(); ++reg_it) {
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getDiskSize();
    }

    if( lmap.originalCtorRegion != NULL ) {
        // Account for original .ctors section (minus the header and trailer)
        retOffset += lmap.originalCtorRegion->getDiskSize() - addressWidth_ - addressWidth_;
    }
    retOffset += addressWidth_;

    return retOffset;

	return 0;
}

bool emitElfStatic::createNewCtorRegion(LinkMap &) {
	return true;
}


Offset emitElfStatic::layoutNewDtorRegion(LinkMap &lmap) {
    /*
     * .dtors sections are processed in forward order on Linux x86. So new
     * .dtors sections need to be placed after the original .dtors section
     */

    Offset retOffset = lmap.dtorRegionOffset;
    retOffset += addressWidth_;

    pair<map<Region *, LinkMap::AllocPair>::iterator, bool> result;
    if( lmap.originalDtorRegion != NULL ) {
        // Account for the original .dtors section (minus the header and trailer)
        retOffset += lmap.originalDtorRegion->getDiskSize() - addressWidth_ - addressWidth_;
    }

    set<Region *, CtorComp<Region*> >::iterator reg_it;
    for(reg_it = lmap.newDtorRegions.begin(); reg_it != lmap.newDtorRegions.end(); ++reg_it) {
        result = lmap.regionAllocs.insert(make_pair(*reg_it, make_pair(0, retOffset)));

        // If the map already contains this Region, this is a logic error
        if( !result.second ) {
            return ~0UL;
        }

        retOffset += (*reg_it)->getDiskSize();
    }

    retOffset += addressWidth_;
    return retOffset;

	return 0;
}

bool emitElfStatic::createNewDtorRegion(LinkMap &) {
	return true;
}

bool emitElfStatic::isConstructorRegion(Region *reg) {
	return ( CTOR_NAME.compare(reg->getRegionName()) == 0 );

}

bool emitElfStatic::isDestructorRegion(Region *reg) {
	return ( DTOR_NAME.compare(reg->getRegionName()) == 0 );
}

bool emitElfStatic::isGOTRegion(Region *reg) {

    if( PPC32_WIDTH == addressWidth_ ) {
	return false;
    }
	return ( TOC_NAME.compare(reg->getRegionName()) == 0 );
}
bool emitElfStatic::isGOTRelocation(unsigned long relType) {
    if( PPC32_WIDTH == addressWidth_ ) {
        switch(relType) {
            case R_PPC_GOT16:
            case R_PPC_GOT16_LO:
            case R_PPC_GOT16_HI:
            case R_PPC_GOT16_HA:
            case R_PPC_GOT_TPREL16:
            case R_PPC_TLS:
                return true;
                break;
            default:
                return false;
                break;
        }
    } else if( PPC64_WIDTH == addressWidth_) {
        switch(relType) {
            case R_PPC64_GOT16:
            case R_PPC64_GOT16_LO:
            case R_PPC64_GOT16_HI:
            case R_PPC64_GOT16_HA:
            case R_PPC64_TLS:
	    case R_PPC64_TOC16_DS:
            case R_PPC64_TOC16:
                return true;
                break;
            default:
                return false;
                break;
        }
    } else{
        assert(!UNKNOWN_ADDRESS_WIDTH_ASSERT);
    }

    return false;
}

void emitElfStatic::getExcludedSymbolNames(set<string> &) {
}

bool emitElfStatic::updateTOC(Symtab *file, LinkMap &lmap, Offset globalOffset) {
  // I don't think we need this now that we're using multiple TOCs. Instead we'll
  // split the GOTs. 
  // DISABLED
  return true;


  rewrite_printf("Updating TOC in %s\n", file->name().c_str());

  Address GOTbase = lmap.gotRegionOffset;

  Address oldTOC = file->getTOCoffset((Offset) 0);
  Address newTOC = GOTbase + 0x8000 + globalOffset; // Convention is to use +8k to split the 
  // reachable distance. TODO: calculate this from the original GOT and TOC value.
  Address deltaTOC = newTOC - oldTOC;

  rewrite_printf("Set TOC to 0x%lx = 0x%lx + 0x%lx + 0x%lx; old 0x%lx\n", newTOC, GOTbase, 0x8000UL, globalOffset, oldTOC);

  // We're doing this _before_ the emitElf64::driver call, so we can make changes
  // to original sections. That's good, because we need to update the OPD section
  // to match our new TOC value. 

  Region *OPD = NULL;
  if (!file->findRegion(OPD, ".opd")) return true;
  
  unsigned long *oldData = (unsigned long *) OPD->getPtrToRawData();
  unsigned long *newData = (unsigned long *) malloc(OPD->getDiskSize());
  memcpy(newData, oldData, OPD->getDiskSize());
  OPD->setPtrToRawData(newData, OPD->getDiskSize());

  // Let's do the thing. 
  // The OPD data structures are 3-tuples of <func ptr, TOC, misc>. We want
  // to update the TOC part of that. So... cleverness.
  for (unsigned i = 1; i < (OPD->getDiskSize() / sizeof(unsigned long)); i += 3) {
    unsigned long oldVal = newData[i];
    newData[i] = oldVal + deltaTOC;
  }

  return true;
}

Offset emitElfStatic::allocStubRegions(LinkMap &lmap, Offset globalOffset) {
  // Count the number of external calls in each Region and alloc
  // 16 (4*insnsize) bytes for each. 
  //
  // 16: stub code pattern is:
  //   std   r2, 40(r1)
  //   addis r2, r2, <hi(new TOC - old TOC)>
  //   addi  r2, r2, <lo(new TOC - old TOC)>
  //   b     <destination>

  // We can get external calls by looking for unique relocation destinations; 
  // any intra-module call would not have a relocation. 
  
  // So. Relocation time. 
  // We can't just count relocations because there are TOC setting relocs and we'll massively overapproximate. 
  // Instead, count of type R_PPC64_REL24. Also, unique. 

  Offset relStubOffset = lmap.stubRegionOffset;
  rewrite_printf("Allocating stub regions for inter-module calls\n");

  for (deque<Region *>::iterator reg_iter = lmap.codeRegions.begin(); reg_iter != lmap.codeRegions.end(); ++reg_iter) {
    vector<relocationEntry> region_rels = (*reg_iter)->getRelocations();
    rewrite_printf("\t For region %s in %s, %lu relocations, %lu unique symbols so far\n",
		   (*reg_iter)->getRegionName().c_str(), 
		   (*reg_iter)->symtab()->name().c_str(),
		   region_rels.size(), 
		   lmap.stubMap.size());
    for (vector<relocationEntry>::iterator rel_iter = region_rels.begin(); rel_iter != region_rels.end(); ++rel_iter) {
      relocationEntry &rel = *rel_iter;
      if (rel.getRelType() != R_PPC64_REL24) continue;
      
      if (lmap.stubMap.find(rel.getDynSym()) == lmap.stubMap.end()) {
	rewrite_printf("\t\tSetting stub offset for %s to 0x%lx\n", rel.getDynSym()->getMangledName().c_str(), relStubOffset);
	lmap.stubMap[rel.getDynSym()] = relStubOffset + globalOffset;
	relStubOffset += 4*sizeof(unsigned);
      }
    }
  }
  rewrite_printf("\t After regions, returning offset 0x%lx = current 0x%lx + stub area 0x%lx\n",
		 lmap.stubRegionOffset + (16 * lmap.stubMap.size()), lmap.stubRegionOffset, 16*lmap.stubMap.size());
  return relStubOffset;
}


bool emitElfStatic::handleInterModuleSpecialCase(Symtab * /*target*/,
						 Symtab * /*src*/,
						 LinkMap &lmap,
						 char *data,
						 relocationEntry rel,
						 Offset newTOC,
						 Offset oldTOC,
						 Offset dest,
						 Offset relOffset,
						 Offset globalOffset) {
  // This isn't that bad :)
  //
  // We have code of the following type:
  //
  // call <target>
  // nop
  //
  // where <target> is encoded in the relocation rel. We also have
  // varying TOC values. 
  //
  // We want:
  //
  // call <stub>
  // ld r2, 40(r1)
  //
  // stub code:
  // 
  // std    r2, 40(r1)
  // addis  r2, r2(hi(newTOC - oldTOC))
  // addi   r2, r2(lo(newTOC - oldTOC))
  // b <target>
  //
  // So this needs to either look up existing stub code and reuse it 
  // or create it. 
  //

  auto *td = reinterpret_cast<unsigned int *>(data + dest - (dest % 4));
  unsigned int &call = td[0];
  unsigned int &post = td[1];

  instruction callInsn(call);
  if (!callInsn.isCall()) {
    // A branch or other non-call/noop
    return false;
  }
  if (post != 0x60000000) {
    // No noop for us to restore the TOC
    return false;
  }

  Offset stubOffset = findOrCreateStub(rel.getDynSym(), lmap, newTOC, oldTOC, data, globalOffset);
  Offset branchOffset = stubOffset - relOffset;
  rewrite_printf("Handling TOC set stub, using stub offset 0x%lx, curr addr 0x%lx so branch is 0x%lx \n", 
		 stubOffset, relOffset, branchOffset);
  rewrite_printf("\t Raw original call is 0x%x...", call);
  // 6 bits in, 24 bits long
  call = setBits(call, 6, 24, (unsigned) (branchOffset));
  rewrite_printf("and patched 0x%x\n", call);
  
  // And set the TOC restore
  // Post better be a noop. Assert for now, return false after devel is done. 
  assert(post == 0x60000000);   
  // And ld r2, 40(r1). Always the same instruction. 
  post = 0xe8410028; 

  return true;
}

Offset emitElfStatic::findOrCreateStub(Symbol *sym, LinkMap &lmap, Offset newTOC, Offset oldTOC, char *data, Offset globalOffset) {
  // Easy case
  std::map<Symbol *, TOCstub>::iterator iter = stubMap.find(sym);
  if (iter != stubMap.end() &&
      getNewTOC(iter->second) == newTOC &&
      getOldTOC(iter->second) == oldTOC) {
    return getStubOffset(iter->second);
  }

  // Otherwise create one
  Offset stubOffset = lmap.stubMap[sym];
  assert(stubOffset);

  unsigned stub[4];
  createStub(stub, stubOffset, newTOC, oldTOC, sym->getOffset());
  
  char *td = data + stubOffset - globalOffset;
  rewrite_printf("Creating stub at 0x%lx (0x%lx in buffer)\n",
		 stubOffset, stubOffset - globalOffset);
  memcpy(td, stub, 4*sizeof(unsigned));
  
  stubMap[sym] = TOCstub(stubOffset, newTOC, oldTOC);
  return stubOffset;
} 

void emitElfStatic::createStub(unsigned *stub, Offset stubOffset, Offset newTOC, Offset oldTOC, Offset dest) {

  unsigned deltaTOChi = HA((newTOC - oldTOC));
  unsigned deltaTOClo = LOW((newTOC - oldTOC));
  unsigned deltaAddr = dest - (stubOffset + 3*sizeof(unsigned));

  rewrite_printf("Calculating stub:\n\thigh TOC bits 0x%x (from 0x%lx - 0x%lx),\n\tlow TOC bits 0x%x (from 0x%lx - 0x%lx),\n\tbranch offset 0x%x (from 0x%lx - 0x%lx)\n",
		 deltaTOChi, newTOC, oldTOC,
		 deltaTOClo, newTOC, oldTOC,
		 deltaAddr, dest, stubOffset);

  stub[0] = 0xf8410028; // std
  stub[1] = 0x3c420000 | (deltaTOChi & 0xffff);
  stub[2] = 0x38420000 | (deltaTOClo & 0xffff);

  instruction branch;
  IFORM_OP_SET(branch, Bop);
  IFORM_LI_SET(branch, deltaAddr >> 2);
  IFORM_AA_SET(branch, 0);
  IFORM_LK_SET(branch, 0);

  stub[3] = branch.asInt();
  
}

bool emitElfUtils::updateRelocation(Symtab *obj, relocationEntry &rel, Offset address_adjust) {
    // Currently, only verified on x86 and x86_64 -- this may work on other architectures
    Region *targetRegion = obj->findEnclosingRegion(rel.rel_addr());
    if( NULL == targetRegion ) {
        rewrite_printf("Failed to find enclosing Region for relocation");
        return false;
    }

    // Here is ppc32 so should be addressWidth==4, and other case macros
    // But until this function was moved, it didnt have a case for 32 bit
    unsigned addressWidth = obj->getAddressWidth();
    if( addressWidth == 8 ) {
        switch(rel.getRelType()) {
            case R_PPC64_IRELATIVE:
            case R_PPC64_RELATIVE:
                rel.setAddend(rel.addend() + address_adjust);
                break;
            default:
                break;
        }
    }

    // XXX The GOT also holds a pointer to the DYNAMIC segment -- this is currently not
    // updated. However, this appears to be unneeded for regular shared libraries.

    // From the SYS V ABI x86 supplement
    // "The table's entry zero is reserved to hold the address of the dynamic structure,
    // referenced with the symbol _DYNAMIC. This allows a program, such as the
    // dynamic linker, to find its own dynamic structure without having yet processed
    // its relocation entries. This is especially important for the dynamic linker, because
    // it must initialize itself without relying on other programs to relocate its memory
    // image."

    // In order to implement this, would have determine the final address of a new .dynamic
    // section before outputting the patched GOT data -- this will require some refactoring.

    return true;
}
