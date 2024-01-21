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

#if !defined(_Object_nt_h_)
#define _Object_nt_h_

#include <map>
#include <set>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include "symtabAPI/h/symutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>

#if !defined(__out_ecount_opt)
#define __out_ecount_opt(UNUSED)
#endif
#include <dbghelp.h>

namespace Dyninst{
namespace SymtabAPI{

class ExceptionBlock;

class Object : public AObject
{
	friend class Symtab;
 public:
    class intSymbol
	{
	private:
            std::string name;
            DWORD64 addr;
            DWORD type;
            DWORD linkage;
            DWORD size;
            Region *region;

	public:
            intSymbol( std::string _name,
                       DWORD64 _addr,
                       DWORD _type,
                       DWORD _linkage,
                       DWORD _size,
                       Region *_region)
                : name(_name),
                addr(_addr),
                type(_type),
                linkage(_linkage),
                size(_size),
                region(_region)
		{}

            std::string GetName( void ) const          { return name; }
            DWORD64 GetAddr( void ) const           { return addr; }
            DWORD	GetSize( void ) const				{ return size; }
            DWORD	GetType( void ) const				{ return type; }
            DWORD	GetLinkage( void ) const			{ return linkage; }
            Region *GetRegion( void ) const        { return region; }
            void	SetSize( DWORD cb )					{ size = cb; }

            void DefineSymbol( dyn_hash_map<std::string, std::vector< Symbol *> >& syms,
                               std::map<Symbol *, std::string> &symsToMods,
                               const std::string& modName ) const;
	};

    class File
	{
	private:
            std::string name;
            std::vector<intSymbol*> syms;

	public:
            File( std::string _name = "" )
                : name(_name)
		{}

            void AddSymbol( intSymbol* pSym )
		{
                    syms.push_back( pSym );
		}

            void DefineSymbols( dyn_hash_map<std::string, std::vector< Symbol *> >& syms,
                                std::map<Symbol *, std::string> &symsToMods,
                                const std::string& modName ) const;
            std::string GetName( void ) const		{ return name; }
            const std::vector<intSymbol*>& GetSymbols( void )	const		{ return syms; }
	};

    class Module
	{
	private:
            std::string name;
            DWORD64 baseAddr;
            DWORD64 extent;
            bool isDll;

            std::vector<File *> files;
            File* defFile;

            void PatchSymbolSizes( const Object* obj,
                                   const std::vector<intSymbol*>& allSyms ) const;

	public:
            Module( std::string name,
                    DWORD64 baseAddr,
                    DWORD64 extent = 0 );

            File* GetDefaultFile( void )			{ return defFile; }
            File* FindFile( std::string name );
            void AddFile( File* pFile )				{ files.push_back( pFile ); }

            void DefineSymbols( const Object* obj,
                                dyn_hash_map<std::string, std::vector< Symbol *> > & syms,
                                std::map<Symbol *, std::string> &symsToMods ) const;
            void BuildSymbolMap( const Object* obj ) const; 

            std::string GetName( void ) const            { return name; }
            bool IsDll( void ) const                { return isDll; }
            void SetIsDll( bool v )                 { isDll = v; }
	};

 private:
    Module* curModule;

    Object(const Object &);
    const Object& operator=(const Object &);

 public:
    DYNINST_EXPORT Object(MappedFile *, bool defensive, 
                         void (*)(const char *) = log_msg, bool alloc_syms = true, Symtab* st = NULL);
  
    DYNINST_EXPORT virtual ~Object( void );
	DYNINST_EXPORT std::string getFileName() const { return mf->filename(); }
    DYNINST_EXPORT bool isForwarded( Offset addr );
    DYNINST_EXPORT bool isEEL() const { return false; }
    DYNINST_EXPORT bool isText( const Offset addr ) const; 
    DYNINST_EXPORT Offset get_base_addr() const { return (Offset)mf->base_addr();} 
    DYNINST_EXPORT Module* GetCurrentModule( void )				    { return curModule; }
   
    DYNINST_EXPORT bool getCatchBlock(ExceptionBlock &b, Offset addr, unsigned size = 0) const;
    DYNINST_EXPORT unsigned int GetTextSectionId( void ) const         { return textSectionId;}
    DYNINST_EXPORT PIMAGE_NT_HEADERS   GetImageHeader( void ) const    { return peHdr; }
    DYNINST_EXPORT PVOID GetMapAddr( void ) const                      { return mf->base_addr(); }
    DYNINST_EXPORT Offset getEntryPoint( void ) const                {
		if (peHdr) return peHdr->OptionalHeader.AddressOfEntryPoint;
		return 0;}
    //+ desc.loadAddr(); } //laodAddr is always zero in our fake address space.
    // TODO. Change these later.
    DYNINST_EXPORT Offset getLoadAddress() const { return imageBase; }
	DYNINST_EXPORT Offset getPreferedBase() const { return preferedBase; }
    DYNINST_EXPORT Offset getEntryAddress() const { return getEntryPoint(); }
    DYNINST_EXPORT Offset getBaseAddress() const { return get_base_addr(); }
    DYNINST_EXPORT Offset getTOCoffset(Offset /*ignored*/) const { return 0; }
    DYNINST_EXPORT ObjectType objType() const;
    DYNINST_EXPORT const char *interpreter_name() const { return NULL; }
    DYNINST_EXPORT dyn_hash_map <std::string, LineInformation> &getLineInfo();
    DYNINST_EXPORT void parseTypeInfo();
    DYNINST_EXPORT virtual Dyninst::Architecture getArch() const;
    DYNINST_EXPORT void    ParseGlobalSymbol(PSYMBOL_INFO pSymInfo);
    DYNINST_EXPORT const std::vector<Offset> &getPossibleMains() const   { return possible_mains; }
    DYNINST_EXPORT void getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *mod_langs);
    DYNINST_EXPORT bool emitDriver(std::string fName, std::set<Symbol*> &allSymbols, unsigned flag);
    DYNINST_EXPORT unsigned int getSecAlign() const {return SecAlignment;}
    DYNINST_EXPORT void insertPrereqLibrary(std::string lib);
    virtual char *mem_image() const 
    {
        assert(mf);
        return (char *)mf->base_addr();
    }
    void setTrapHeader(Offset ptr);
    Offset trapHeader();

    DYNINST_EXPORT DWORD ImageOffset2SectionNum(DWORD dwRO);
    DYNINST_EXPORT PIMAGE_SECTION_HEADER ImageOffset2Section(DWORD dwRO);
    DYNINST_EXPORT PIMAGE_SECTION_HEADER ImageRVA2Section(DWORD dwRVA);
    DYNINST_EXPORT DWORD RVA2Offset(DWORD dwRVA);
    DYNINST_EXPORT DWORD Offset2RVA(DWORD dwRO);
    DYNINST_EXPORT void addReference(Offset, std::string, std::string);
    DYNINST_EXPORT std::map<std::string, std::map<Offset, std::string> > & getRefs() { return ref; }

    std::vector<std::pair<std::string, IMAGE_IMPORT_DESCRIPTOR> > & getImportDescriptorTable();
    std::map<std::string, std::map<std::string, WORD> > & getHintNameTable();
    PIMAGE_NT_HEADERS getPEHdr() { return peHdr; }
	void setTOCoffset(Offset) {};
	DYNINST_EXPORT void rebase(Offset off);
	DYNINST_EXPORT Region* findRegionByName(const std::string& name) const;
	DYNINST_EXPORT void applyRelocs(Region* relocs, Offset delta);
	DYNINST_EXPORT virtual void getSegmentsSymReader(std::vector<SymSegment> &);

private:
    DYNINST_EXPORT void    ParseSymbolInfo( bool );
    DYNINST_EXPORT void parseFileLineInfo();
    DYNINST_EXPORT void parseLineInfoForAddr(Offset)
    {
      parseFileLineInfo();
    }
    
    DYNINST_EXPORT void    FindInterestingSections( bool, bool );
    Region *          findEnclosingRegion(const Offset where);
    void AddTLSFunctions();
	DWORD* get_dword_ptr(Offset rva);
    Offset baseAddr;
	Offset preferedBase;
    Offset imageBase;
    PIMAGE_NT_HEADERS   peHdr;
    Offset trapHeaderPtr_;
	unsigned int SecAlignment;

    std::vector<std::pair<std::string, IMAGE_IMPORT_DESCRIPTOR> > idt_;
    std::map<std::string, std::map<std::string, WORD> > hnt_;

   std::map<std::string,std::map<Offset, std::string> > ref;

	unsigned int textSectionId;
	unsigned int dataSectionId;
   
	HANDLE  hProc;
    std::vector<Offset> possible_mains;
};

#if !defined(SYMFLAG_FUNCTION)
#  define SYMFLAG_FUNCTION      SYMF_FUNCTION
#  define SYMFLAG_LOCAL         SYMF_LOCAL
#  define SYMFLAG_PARAMETER     SYMF_PARAMETER
#  define SYMFLAG_EXPORT        SYMF_EXPORT
#endif // !defined(SYMFLAG_FUNCTION)

}//namespace Dyninst
}//namespace SymtabAPI

#endif /* !defined(_Object_nt_h_) */
