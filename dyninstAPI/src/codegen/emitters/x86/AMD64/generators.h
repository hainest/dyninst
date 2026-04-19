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

#ifndef DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_X86_AMD64_GENERATORS_H
#define DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_X86_AMD64_GENERATORS_H

#include "codegen.h"
#include "codegen/emitters/x86/AMD64/generators/xmm.h"
#include "codegen/emitters/x86/generators.h"
#include "dyn_register.h"

namespace Dyninst { namespace DyninstAPI { namespace AMD64 {

  void emitAddRM64(Register dest, int imm, bool is_64, codeGen &gen);

  void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen);

  void emitMovImmToRM64(Register base, int disp, int imm, bool is_64, codeGen &gen);

  void emitMovPCRMToReg64(Register dest, int offset, int size, codeGen &gen, bool deref_result);

  void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen);

  void emitMovRegToRM64(Register base, int disp, Register src, int size, codeGen &gen);

  void emitMovRMToReg64(Register dest, Register base, int disp, int size, codeGen &gen);

  void emitMovSegRMToReg64(Register dest, Register base, int disp, codeGen &gen);

  void emitOpMemImm64(unsigned opcode, unsigned opcode_ext, Register base, int imm, bool is_64,
                      codeGen &gen);

  void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm, bool is_64,
                      codeGen &gen);

  void emitOpRegImm8_64(unsigned opcode, unsigned opcode_ext, Register dest, char imm, bool is_64,
                        codeGen &gen);

  void emitOpRegReg64(unsigned opcode, Register dest, Register src, bool is_64, codeGen &gen);

  void emitOpRegRegImm64(unsigned opcode, Register dest, Register src1, int imm, bool is_64,
                         codeGen &gen);

  void emitOpRegRM64(unsigned opcode, Register dest, Register base, int disp, bool is_64,
                     codeGen &gen);

  void emitPopReg64(Register dest, codeGen &gen);

  void emitPushReg64(Register src, codeGen &gen);

  void emitRex(bool is_64, Register *r, Register *x, Register *b, codeGen &gen);

  bool isImm64bit(Address imm);

}}}

#endif
