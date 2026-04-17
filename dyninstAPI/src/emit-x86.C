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
 * emit-x86.C - x86 & AMD64 code generators
 * $Id: emit-x86.C,v 1.64 2008/09/11 20:14:14 mlam Exp $
 */

#include <assert.h>
#include <stdio.h>
#include "codegen/RegControl.h"
#include "compiler_annotations.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/debug.h"
#include "ast.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "registerSpace.h"
#include "arch-regs-x86.h"
#include "dyninstAPI/src/dynProcess.h"

#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/image.h"
// get_index...
#include "dyninstAPI/src/dynThread.h"
#include "ABI.h"
#include "liveness.h"
#include "RegisterConversion.h"
#include "unaligned_memory_access.h"
#include "codegen/emitters/x86/generators.h"
#include "codegen/emitters/x86/AMD64/generators.h"

//
// 64-bit code generation helper functions
//

bool EmitterAMD64Dyn::emitCallInstruction(codeGen &gen, func_instance *callee, Register) {
   // make the call (using an indirect call)
   //emitMovImmToReg64(REGNUM_EAX, callee->addr(), true, gen);
   //emitSimpleInsn(0xff, gen); // group 5
   //emitSimpleInsn(0xd0, gen); // mod = 11, reg = 2 (call Ev), r/m = 0 (RAX)

   
   if (gen.startAddr() != (Address) -1) {
      signed long disp = callee->addr() - (gen.currAddr() + 5);
      int disp_i = (int) disp;
      if (disp == (signed long) disp_i) {
         emitCallRel32(disp_i, gen);
         return true;
      }
   }
   
   std::vector<Register> excluded;
   excluded.push_back(REGNUM_RAX);
   
   Register ptr = gen.rs()->getScratchRegister(gen, excluded);
   gen.markRegDefined(ptr);
   Register effective = ptr;
   Dyninst::DyninstAPI::AMD64::emitMovImmToReg64(ptr, callee->addr(), true, gen);
   if(ptr >= REGNUM_R8) {
     Dyninst::DyninstAPI::AMD64::emitRex(false, NULL, NULL, &effective, gen);
   }
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xFF);
   append_memory_as_byte(insn, static_cast<uint8_t>(0xD0 | effective));
   SET_PTR(insn, gen);

   return true;
}


bool EmitterAMD64Stat::emitCallInstruction(codeGen &gen, func_instance *callee, Register) {
   //fprintf(stdout, "at emitCallInstruction: callee=%s\n", callee->prettyName().c_str());

   AddressSpace *addrSpace = gen.addrSpace();
   Address dest;

   // find func_instance reference in address space
   // (refresh func_map)
   std::vector<func_instance *> funcs;
   addrSpace->findFuncsByAll(callee->prettyName(), funcs);

   // test to see if callee is in a shared module
   assert(gen.func());
   if (gen.func()->obj() != callee->obj()) {
      emitPLTCall(callee, gen);
   } else {
      dest = callee->addr();
      signed long disp = dest - (gen.currAddr() + 5);
      int disp_i = (int) disp;
      assert(disp == (signed long) disp_i);
      emitCallRel32(disp_i, gen);
      return true;
   }
   return true;
}

bool EmitterAMD64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
   // create or retrieve jump slot
   Address dest = getInterModuleFuncAddr(callee, gen);
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xFF);
   // Note: this is a combination of 00 (MOD), 100 (opcode extension), and 101
   // (disp32)
   append_memory_as_byte(insn, 0x25);
   int64_t offset = dest - (gen.currAddr() + sizeof(int32_t) + 2);
   assert(numeric_limits<int32_t>::lowest() <= offset && offset <= numeric_limits<int32_t>::max() && "offset more than 32 bits");
   append_memory_as(insn, static_cast<int32_t>(offset));
   SET_PTR(insn, gen);
   return true;
}

bool EmitterAMD64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
   // create or retrieve jump slot
   Address dest = getInterModuleFuncAddr(callee, gen);
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xFF);
   append_memory_as_byte(insn, 0x15);
   int64_t offset = dest - (gen.currAddr() + sizeof(int32_t) + 2);
   assert(numeric_limits<int32_t>::lowest() <= offset && offset <= numeric_limits<int32_t>::max() && "offset more than 32 bits");
   append_memory_as(insn, static_cast<int32_t>(offset));
   SET_PTR(insn, gen);
   return true;
}
