#include "codegen/emitters/x86/generators.h"
#include "codegen/emitters/x86/generators/mov_register.h"
#include "unaligned_memory_access.h"

namespace Dyninst { namespace DyninstAPI { namespace x86 {

  void emitMovImmToReg(RealRegister dest, int imm, codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xB8 + dest.reg());
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
  }

  void emitMovRegToReg(RealRegister dest, RealRegister src, codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x8B);
    append_memory_as_byte(insn, makeModRMbyte(3, dest.reg(), src.reg()));
    SET_PTR(insn, gen);
  }

}}}
