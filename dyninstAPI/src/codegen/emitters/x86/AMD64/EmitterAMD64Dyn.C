#include "arch-regs-x86.h"
#include "codegen/emitters/x86/AMD64/EmitterAMD64Dyn.h"
#include "codegen/emitters/x86/AMD64/generators.h"
#include "function.h"
#include "registerSpace/registerSpace.h"

namespace Dyninst { namespace DyninstAPI {

  bool EmitterAMD64Dyn::emitCallInstruction(codeGen &gen, func_instance *callee, Register) {
    // make the call (using an indirect call)
    // emitMovImmToReg64(REGNUM_EAX, callee->addr(), true, gen);
    // emitSimpleInsn(0xff, gen); // group 5
    // emitSimpleInsn(0xd0, gen); // mod = 11, reg = 2 (call Ev), r/m = 0 (RAX)

    if(gen.startAddr() != (Address)-1) {
      signed long disp = callee->addr() - (gen.currAddr() + 5);
      int disp_i = (int)disp;
      if(disp == (signed long)disp_i) {
        emitCallRel32(disp_i, gen);
        return true;
      }
    }

    std::vector<Register> excluded;
    excluded.push_back(REGNUM_RAX);

    Register ptr = gen.rs()->getScratchRegister(gen, excluded);
    gen.markRegDefined(ptr);
    Register effective = ptr;
    AMD64::emitMovImmToReg64(ptr, callee->addr(), true, gen);
    if(ptr >= REGNUM_R8) {
      AMD64::emitRex(false, NULL, NULL, &effective, gen);
    }
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xFF);
    append_memory_as_byte(insn, static_cast<uint8_t>(0xD0 | effective));
    SET_PTR(insn, gen);

    return true;
  }

}}
