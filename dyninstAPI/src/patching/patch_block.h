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

#ifndef DYNINST_DYNINSTAPI_PATCHING_PATCH_BLOCK_H
#define DYNINST_DYNINSTAPI_PATCHING_PATCH_BLOCK_H

#include "PatchCFG.h"
#include "addressSpace.h"
#include "dyntypes.h"
#include "mapped_object.h"
#include "parsing/parse_block.h"

#include <set>
#include <string>
#include <vector>

class func_instance;

namespace Dyninst { namespace DyninstAPI {

  class patch_edge;

  class patch_block : public PatchAPI::PatchBlock {
    friend class mapped_object;

  public:
    patch_block(ParseAPI::Block *ib, mapped_object *obj);

    patch_block(const patch_block *parent, mapped_object *child);

    ~patch_block() = default;

    // Up-accessors
    mapped_object *obj() const {
      return SCAST_MO(obj_);
    }

    AddressSpace *addrSpace() const;

    AddressSpace *proc() const {
      return addrSpace();
    }

    template <class OutputIterator>
    void getFuncs(OutputIterator result) {
      std::vector<ParseAPI::Function *> pFuncs;
      llb()->getFuncs(pFuncs);
      for (unsigned i = 0; i < pFuncs.size(); ++i) {
        func_instance *func = findFunction(pFuncs[i]);
        *result = func;
        ++result;
      }
    }

    void triggerModified();

    void setNotAbruptEnd();

    parse_block *llb() const {
      return SCAST_PB(block_);
    }

    void *getPtrToInstruction(Address addr) const;

    patch_edge *getTarget();

    patch_edge *getFallthrough();

    // NULL if not conclusive
    patch_block *getFallthroughBlock();

    func_instance *callee();

    std::string calleeName();

    bool _ignorePowerPreamble;

    int id() const;

    // Functions to avoid
    // These are convinence wrappers for really expensive
    // lookups, and thus should be avoided.
    func_instance *entryOfFunc() const;

    bool isFuncExit() const;

    Address GetBlockStartingAddress();

    void markModified();

  private:
    void updateCallTarget(func_instance *func);

    func_instance *findFunction(ParseAPI::Function *);

    func_instance *callee(std::string const &);
  };

}}

#endif
