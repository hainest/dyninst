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

#ifndef _CFG_MODIFIER_H_
#define _CFG_MODIFIER_H_

#include <vector>
#include "dyntypes.h"

#include "CodeSource.h"


namespace Dyninst {
namespace ParseAPI {
   class Edge;
   class Block;
   class Function;
   class CodeObject;

   class InsertedRegion;

class CFGModifier {
  public:

   DYNINST_EXPORT static bool redirect(Edge *edge, Block *target);

   DYNINST_EXPORT static Block *split(Block *, Address, bool trust = false, Address newlast = -1);
   
   DYNINST_EXPORT static InsertedRegion *insert(CodeObject *obj, 
                                               Address base, void *data, 
                                               unsigned size);

   DYNINST_EXPORT static bool remove(std::vector<Block *> &, bool force = false);

   DYNINST_EXPORT static bool remove(Function *);

   DYNINST_EXPORT static Function *makeEntry(Block *);
};

class InsertedRegion : public CodeRegion {
  public:
   
   DYNINST_EXPORT InsertedRegion(Address base, void *data, unsigned size, Architecture arch); 
   DYNINST_EXPORT virtual ~InsertedRegion();

   DYNINST_EXPORT Address low() const { return base_; }
   DYNINST_EXPORT Address high() const { return base_ + size_; }

   DYNINST_EXPORT bool isValidAddress(const Address a) const { 
      return (a >= low() && a < high());
   }
   DYNINST_EXPORT void* getPtrToInstruction(const Address a) const {
      if (!isValidAddress(a)) return NULL;
      return (void *)((char *)buf_ + (a - base_));
   }
   DYNINST_EXPORT void* getPtrToData(const Address) const {
      return NULL; 
   }
   DYNINST_EXPORT bool isCode(const Address a) const { return isValidAddress(a); }
   DYNINST_EXPORT bool isData(const Address) const { return false; }
   DYNINST_EXPORT bool isReadOnly(const Address) const { return false; }
   DYNINST_EXPORT Address offset() const { return base_; }
   DYNINST_EXPORT Address length() const { return size_; }
   DYNINST_EXPORT unsigned int getAddressWidth() const {
      if (arch_ == Arch_ppc64 || arch_ == Arch_x86_64 || arch_ == Arch_aarch64) return 8;
      else return 4;
   }
   DYNINST_EXPORT Architecture getArch() const { return arch_; }

   DYNINST_EXPORT bool wasUserAdded() const { return true; }

  private:
    Address base_;
    void *buf_;
    unsigned size_;
    Architecture arch_;
};

}
}

#endif
   
