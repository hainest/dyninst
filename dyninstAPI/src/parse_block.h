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

#ifndef DYNINSTAPI_PARSE_BLOCK_H
#define DYNINSTAPI_PARSE_BLOCK_H

#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "codeRange.h"
#include "dyn_register.h"
#include "dyntypes.h"

class parse_func;

class parse_block : public codeRange, public Dyninst::ParseAPI::Block {
  friend class parse_func;
  friend class DynCFGFactory;

  parse_block(Dyninst::ParseAPI::CodeObject *obj,
              Dyninst::ParseAPI::CodeRegion *reg, Dyninst::Address addr)
      : Dyninst::ParseAPI::Block(obj, reg, addr) {}

public:
  parse_block(parse_func *, Dyninst::ParseAPI::CodeRegion *, Dyninst::Address);
  ~parse_block() = default;

  // just pass through to Block
  Dyninst::Address firstInsnOffset() const;
  Dyninst::Address lastInsnOffset() const;
  Dyninst::Address endOffset() const;
  Dyninst::Address getSize() const;

  // cfg access & various predicates
  bool isShared() const { return containingFuncs() > 1; }
  bool isExitBlock();
  bool isCallBlock();
  bool isIndirectTailCallBlock();
  bool isEntryBlock(parse_func *f) const;
  parse_func *getEntryFunc() const; // func starting with this bock

  bool unresolvedCF() const { return unresolvedCF_; }
  bool abruptEnd() const { return abruptEnd_; }
  void setUnresolvedCF(bool newVal);
  void setAbruptEnd(bool newVal) { abruptEnd_ = newVal; }

  // misc utility
  int id() const { return blockNumber_; }
  void debugPrint();
  image *img();

  // Find callees
  parse_func *getCallee();

  // Returns the address of our callee (if we're a call block, of course)
  std::pair<bool, Dyninst::Address> callTarget();

  // instrumentation-related
  bool needsRelocation() const { return needsRelocation_; }
  void markAsNeedingRelocation() { needsRelocation_ = true; }

  // codeRange implementation
  void *getPtrToInstruction(Dyninst::Address addr) const;
  Dyninst::Address get_address() const { return firstInsnOffset(); }
  unsigned get_size() const { return getSize(); }

  struct compare {
    bool operator()(parse_block *const &b1, parse_block *const &b2) const {
      if (b1->firstInsnOffset() < b2->firstInsnOffset())
        return true;
      if (b2->firstInsnOffset() < b1->firstInsnOffset())
        return false;

      return false;
    }
  };

  // The provided parameter is a magic offset to add to each instruction's
  // address; we do this to avoid a copy when getting Insns from block_instances
  void getInsns(Dyninst::ParseAPI::Block::Insns &instances,
                Dyninst::Address offset = 0);

private:
  using Dyninst::ParseAPI::Block::getInsns;
  bool needsRelocation_{};
  int blockNumber_{};
  bool unresolvedCF_{false};
  bool abruptEnd_{false};
};

#endif
