#include "arch-regs-x86.h"
#include "codegen/emitters/x86/AMD64/generators/mov_register.h"
#include "codegen/emitters/x86/AMD64/generators/prefix.h"
#include "codegen/emitters/x86/generators.h"
#include "registerSpace/RealRegister.h"
#include "unaligned_memory_access.h"

namespace Dyninst { namespace DyninstAPI { namespace AMD64 {

  void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen) {
    Register tmp_dest = dest;
    gen.markRegDefined(dest);
    emitRex(is_64, NULL, NULL, &tmp_dest, gen);
    if(is_64) {
      GET_PTR(insn, gen);
      append_memory_as_byte(insn, 0xB8 + tmp_dest);
      append_memory_as(insn, int64_t{imm});
      SET_PTR(insn, gen);
    } else {
      x86::emitMovImmToReg(RealRegister(tmp_dest), imm, gen);
    }
  }

}}}
