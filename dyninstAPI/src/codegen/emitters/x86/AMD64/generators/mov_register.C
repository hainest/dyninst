#include "arch-regs-x86.h"
#include "codegen/emitters/x86/AMD64/generators/mov_register.h"
#include "codegen/emitters/x86/AMD64/generators/prefix.h"
#include "codegen/emitters/x86/generators.h"
#include "registerSpace/RealRegister.h"
#include "unaligned_memory_access.h"

namespace Dyninst { namespace DyninstAPI { namespace AMD64 {

  void emitMovPCRMToReg64(Register dest, int offset, int size, codeGen &gen, bool deref_result) {
    GET_PTR(insn, gen);
    if(size == 8) {
      append_memory_as_byte(insn, (dest & 0x8) >> 1 | 0x48); // REX prefix
    } else {
      append_memory_as_byte(insn, (dest & 0x8) >> 1 | 0x40); // REX prefix
    }
    if(deref_result) {
      append_memory_as_byte(insn, 0x8B); // MOV instruction
    } else {
      append_memory_as_byte(insn, 0x8D); // LEA instruction
    }
    append_memory_as_byte(insn, ((dest & 0x7) << 3) | 0x5); // ModRM byte
    append_memory_as(insn, int32_t{offset - 7});            // offset
    gen.markRegDefined(dest);
    SET_PTR(insn, gen);
  }

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

  void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen) {
    if(dest == src) {
      return;
    }

    Register tmp_dest = dest;
    Register tmp_src = src;
    emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
    x86::emitMovRegToReg(RealRegister(tmp_dest), RealRegister(tmp_src), gen);
    gen.markRegDefined(dest);
  }

  void emitMovRMToReg64(Register dest, Register base, int disp, int size, codeGen &gen) {
    Register tmp_dest = dest;
    Register tmp_base = base;

    gen.markRegDefined(dest);
    if(size == 1 || size == 2) {
      emitRex(true, &tmp_dest, NULL, &tmp_base, gen);
      GET_PTR(insn, gen);
      append_memory_as_byte(insn, 0x0f);
      if(size == 1) {
        append_memory_as_byte(insn, 0xb6);
      } else if(size == 2) {
        append_memory_as_byte(insn, 0xb7);
      }
      SET_PTR(insn, gen);
      x86::emitAddressingMode(tmp_base, 0, tmp_dest, gen);
    }
    if(size == 4 || size == 8) {
      emitRex((size == 8), &tmp_dest, NULL, &tmp_base, gen);
      emitMovRMToReg(RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
    }
  }

}}}
