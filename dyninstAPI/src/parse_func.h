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

#ifndef DYNINSTAPI_PARSE_FUNC_H
#define DYNINSTAPI_PARSE_FUNC_H

#include "CFG.h"
#include "CodeSource.h"
#include "InstructionSource.h"
#include "image.h"
#include "parRegion.h"

#include <string>
#include <vector>

class parse_func : public Dyninst::ParseAPI::Function {
  friend class DynCFGFactory;
  friend class DynParseCallback;

public:
  parse_func() {}

  parse_func(Dyninst::SymtabAPI::Function *func, pdmodule *m, image *i,
             Dyninst::ParseAPI::CodeObject *obj,
             Dyninst::ParseAPI::CodeRegion *reg,
             Dyninst::InstructionSource *isrc,
             Dyninst::ParseAPI::FuncSource src);

  ~parse_func() = default;

  Dyninst::SymtabAPI::Function *getSymtabFunction() const { return func_; }

  /*** Function naming ***/
  std::string symTabName() const {
    return func_->getFirstSymbol()->getMangledName();
  }
  std::string prettyName() const {
    return func_->getFirstSymbol()->getPrettyName();
  }
  std::string typedName() const {
    return func_->getFirstSymbol()->getTypedName();
  }

  Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_begin() const {
    return func_->mangled_names_begin();
  }
  Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_end() const {
    return func_->mangled_names_end();
  }
  Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_begin() const {
    return func_->pretty_names_begin();
  }
  Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_end() const {
    return func_->pretty_names_end();
  }
  Dyninst::SymtabAPI::Aggregate::name_iter typed_names_begin() const {
    return func_->typed_names_begin();
  }
  Dyninst::SymtabAPI::Aggregate::name_iter typed_names_end() const {
    return func_->typed_names_end();
  }

  void copyNames(parse_func *duplicate);

  // return true if the name is new (and therefore added)
  bool addSymTabName(std::string name, bool isPrimary = false);
  bool addPrettyName(std::string name, bool isPrimary = false);
  bool addTypedName(std::string name, bool isPrimary = false);

  /*** Location queries ***/
  Address getOffset() const;
  Address getPtrOffset() const;
  unsigned getSymTabSize() const;
  Address getEndOffset(); // May trigger parsing

  void *getPtrToInstruction(Address addr) const;

  /*** misc. accessors ***/
  pdmodule *pdmod() const { return mod_; }
  image *img() const { return image_; }

  // Initiate parsing on this function
  bool parse();

  const std::vector<image_parRegion *> &parRegions();

  bool isInstrumentable();
  bool hasUnresolvedCF();

  // ----------------------------------------------------------------------

  ///////////////////////////////////////////////////
  // Mutable function code, used for hybrid analysis
  ///////////////////////////////////////////////////

  void getReachableBlocks(const std::set<parse_block *> &exceptBlocks, // input
                          const std::list<parse_block *> &seedBlocks,  // input
                          std::set<parse_block *> &reachableBlocks);   // output

  // only call on defensive binaries
  ParseAPI::FuncReturnStatus init_retstatus() const;

  // also sets retstatus
  void setinit_retstatus(ParseAPI::FuncReturnStatus rs);

  // true if we stopped the parse at a weird instruction (e.g., arpl)
  bool hasWeirdInsns() { return hasWeirdInsns_; }
  void setHasWeirdInsns(bool wi);

  void setPrevBlocksUnresolvedCF(size_t newVal) {
    prevBlocksUnresolvedCF_ = newVal;
  }
  size_t getPrevBlocksUnresolvedCF() const { return prevBlocksUnresolvedCF_; }

  // ----------------------------------------------------------------------

  ////////////////////////////////////////////////
  // Misc
  ////////////////////////////////////////////////

  struct compare {
    bool operator()(parse_func *const &f1, parse_func *const &f2) const {
      return (f1->getOffset() < f2->getOffset());
    }
  };

#if defined(DYNINST_HOST_ARCH_POWER) || defined(DYNINST_HOST_ARCH_AARCH64)
  bool savesReturnAddr() const { return saves_return_addr_; }
#endif

  parse_block *entryBlock();

  bool isPLTFunction();

  bool isLeafFunc();

  const SymtabAPI::Function *func() const { return func_; }

  bool containsPowerPreamble() { return containsPowerPreamble_; }
  void setContainsPowerPreamble(bool c) { containsPowerPreamble_ = c; }
  parse_func *getNoPowerPreambleFunc() { return noPowerPreambleFunc_; }
  void setNoPowerPreambleFunc(parse_func *f) { noPowerPreambleFunc_ = f; }
  Address getPowerTOCBaseAddress() { return baseTOC_; }
  void setPowerTOCBaseAddress(Address addr) { baseTOC_ = addr; }

private:
  enum class UnresolvedCF { UNSET_CF, HAS_UNRESOLVED_CF, NO_UNRESOLVED_CF };

  /* pointer to the underlying symtab Function */
  SymtabAPI::Function *func_{nullptr};

  /* pointer to file that defines func. */
  pdmodule *mod_{nullptr};
  image *image_{nullptr};

  //  OpenMP (and other parallel language) support
  /* vector of all parallel regions within function */
  std::vector<image_parRegion *> parRegionsList;
  void addParRegion(Dyninst::Address begin, Dyninst::Address end, parRegType t);
  // End OpenMP support

  bool hasWeirdInsns_{false};

  // num func blocks when calculated
  size_t prevBlocksUnresolvedCF_{};

  // Some functions are known to be unparesable by name
  bool isInstrumentableByFunctionName();
  UnresolvedCF unresolvedCF_{UnresolvedCF::UNSET_CF};

  ParseAPI::FuncReturnStatus init_retstatus_{ParseAPI::FuncReturnStatus::UNSET};

  // Architecture specific data
  bool saves_return_addr_{false};

  bool isPLTFunction_{false};

  bool containsPowerPreamble_{false};

  // If the function contains the power preamble, this field points the
  // corresponding function that does not contain the preamble
  parse_func *noPowerPreambleFunc_{nullptr};

  Address baseTOC_{};
};

#endif
