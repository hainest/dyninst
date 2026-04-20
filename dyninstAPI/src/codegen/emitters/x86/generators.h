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

#ifndef DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_X86_GENERATORS_H
#define DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_X86_GENERATORS_H

/*
 *  Generic generators for IA32 and AMD64
 */

#include "codegen.h"
#include "codegen/emitters/x86/generators/mov_register.h"
#include "dyntypes.h"
#include "registerSpace/RealRegister.h"

namespace Dyninst { namespace DyninstAPI { namespace x86 {

  void emitAddMem(Address addr, int imm, codeGen &gen);

  void emitAddressingMode(unsigned base, unsigned index, unsigned int scale, Dyninst::RegValue disp,
                          int reg_opcode, codeGen &gen);

  void emitAddressingMode(unsigned base, Dyninst::RegValue disp, unsigned reg_opcode, codeGen &gen);

  void emitCallRel32(unsigned disp32, codeGen &gen);

  void emitSegPrefix(Register segReg, codeGen &gen);

  /* emit a simple one-byte instruction */
  void emitSimpleInsn(unsigned opcode, codeGen &gen);

  /* build the MOD/RM byte of an instruction */
  constexpr inline uint8_t makeModRMbyte(unsigned Mod, unsigned Reg, unsigned RM) {
    return ((Mod & 0x3U) << 6U) + ((Reg & 0x7U) << 3U) + (RM & 0x7U);
  }

  // Build the SIB byte of an instruction
  constexpr inline uint8_t makeSIBbyte(unsigned Scale, unsigned Index, unsigned Base) {
    return ((Scale & 0x3U) << 6U) + ((Index & 0x7U) << 3U) + (Base & 0x7U);
  }

}}}

#endif
