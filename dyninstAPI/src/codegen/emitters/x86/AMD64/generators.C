#include "arch-regs-x86.h"
#include "codegen/emitters/x86/AMD64/generators.h"
#include "registerSpace/registerSpace.h"
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
      emitMovImmToReg(RealRegister(tmp_dest), imm, gen);
    }
  }

  bool isImm64bit(Address imm) {
    return (imm >> 32);
  }

  void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen) {
    if(dest == src) {
      return;
    }

    Register tmp_dest = dest;
    Register tmp_src = src;
    emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
    emitMovRegToReg(RealRegister(tmp_dest), RealRegister(tmp_src), gen);
    gen.markRegDefined(dest);
  }

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
      emitAddressingMode(tmp_base, 0, tmp_dest, gen);
    }
    if(size == 4 || size == 8) {
      emitRex((size == 8), &tmp_dest, NULL, &tmp_base, gen);
      emitMovRMToReg(RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
    }
  }

  void emitMovSegRMToReg64(Register dest, Register base, int disp, codeGen &gen) {
    Register tmp_dest = dest;
    Register tmp_base = base;

    gen.markRegDefined(dest);

    x86::emitSegPrefix(base, gen);
    emitRex(true, &tmp_dest, NULL, &tmp_base, gen);
    emitOpSegRMReg(MOV_RM32_TO_R32, RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
  }

  void emitMovRegToRM64(Register base, int disp, Register src, int size, codeGen &gen) {
    Register tmp_base = base;
    Register tmp_src = src;
    Register rax = REGNUM_RAX;
    if(size == 1 || size == 2) {
      // mov src, rax
      // mov a[l/x], (dest)
      gen.markRegDefined(REGNUM_RAX);
      if(tmp_src != REGNUM_RAX) {
        emitRex(true, &tmp_src, NULL, &rax, gen);
        emitMovRegToReg(RealRegister(rax), RealRegister(tmp_src), gen);
      }

      // emit prefix
      if(size == 2) {
        x86::emitSimpleInsn(0x66, gen);
      }

      emitRex(false, NULL, NULL, &tmp_base, gen);
      GET_PTR(insn, gen);
      if(size == 1) {
        append_memory_as_byte(insn, 0x88);
      } else if(size == 2) {
        append_memory_as_byte(insn, 0x89);
      }
      SET_PTR(insn, gen);
      emitAddressingMode(tmp_base, 0, REGNUM_RAX, gen);
    }

    if(size == 4 || size == 8) {
      emitRex((size == 8), &tmp_src, NULL, &tmp_base, gen);
      emitMovRegToRM(RealRegister(tmp_base), disp, RealRegister(tmp_src), gen);
    }
  }

  void emitOpRegReg64(unsigned opcode, Register dest, Register src, bool is_64, codeGen &gen) {
    Register tmp_dest = dest;
    Register tmp_src = src;
    emitRex(is_64, &tmp_dest, NULL, &tmp_src, gen);
    emitOpRegReg(opcode, RealRegister(tmp_dest), RealRegister(tmp_src), gen);
    gen.markRegDefined(dest);
  }

  void emitOpRegRM64(unsigned opcode, Register dest, Register base, int disp, bool is_64,
                     codeGen &gen) {
    Register tmp_dest = dest;
    Register tmp_base = base;
    emitRex(is_64, &tmp_dest, NULL, &tmp_base, gen);
    emitOpRegRM(opcode, RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
    gen.markRegDefined(dest);
  }

  void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm, bool is_64,
                      codeGen &gen) {
    Register tmp_rm_reg = rm_reg;
    emitRex(is_64, NULL, NULL, &tmp_rm_reg, gen);

    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    append_memory_as_byte(insn, 0xC0 | ((opcode_ext & 0x7) << 3) | tmp_rm_reg);
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
    gen.markRegDefined(rm_reg);
  }

  // operation on memory location specified with a base register
  // (does not work for RSP, RBP, R12, R13)
  void emitOpMemImm64(unsigned opcode, unsigned opcode_ext, Register base, int imm, bool is_64,
                      codeGen &gen) {
    Register tmp_base = base;
    emitRex(is_64, NULL, NULL, &tmp_base, gen);

    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    append_memory_as_byte(insn, ((opcode_ext & 0x7) << 3) | tmp_base);
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
  }

  void emitOpRegRegImm64(unsigned opcode, Register dest, Register src1, int imm, bool is_64,
                         codeGen &gen) {
    emitOpRegReg64(opcode, dest, src1, is_64, gen);
    GET_PTR(insn, gen);
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
    gen.markRegDefined(dest);
  }

  void emitOpRegImm8_64(unsigned opcode, unsigned opcode_ext, Register dest, char imm, bool is_64,
                        codeGen &gen) {
    Register tmp_dest = dest;
    emitRex(is_64, NULL, NULL, &tmp_dest, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    append_memory_as_byte(insn, 0xC0 | ((opcode_ext & 0x7) << 3) | tmp_dest);
    append_memory_as_byte(insn, imm);
    SET_PTR(insn, gen);
    gen.markRegDefined(dest);
  }

  void emitPushReg64(Register src, codeGen &gen) {
    emitRex(false, NULL, NULL, &src, gen);
    x86::emitSimpleInsn(0x50 + src, gen);
    if(gen.rs()) {
      gen.rs()->incStack(8);
    }
  }

  void emitPopReg64(Register dest, codeGen &gen) {
    emitRex(false, NULL, NULL, &dest, gen);
    x86::emitSimpleInsn(0x58 + dest, gen);
    if(gen.rs()) {
      gen.rs()->incStack(-8);
    }
  }

  void emitMovImmToRM64(Register base, int disp, int imm, bool is_64, codeGen &gen) {
    GET_PTR(insn, gen);
    if(base == Null_Register) {
      append_memory_as_byte(insn, 0xC7);
      append_memory_as_byte(insn, 0x84);
      append_memory_as_byte(insn, 0x25);
      append_memory_as(insn, int32_t{disp});
    } else {
      emitRex(is_64, &base, NULL, NULL, gen);
      append_memory_as_byte(insn, 0xC7);
      SET_PTR(insn, gen);
      emitAddressingMode(base, disp, 0, gen);
      REGET_PTR(insn, gen);
    }
    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
  }

  void emitAddRM64(Register dest, int imm, bool is_64, codeGen &gen) {
    if(imm == 1) {
      emitRex(is_64, &dest, NULL, NULL, gen);
      GET_PTR(insn, gen);
      append_memory_as_byte(insn, 0xFF);
      append_memory_as_byte(insn, dest & 0x7);
      SET_PTR(insn, gen);
      return;
    }
    emitRex(is_64, &dest, NULL, NULL, gen);
    emitOpMemImm64(0x81, 0x0, dest, imm, true, gen);
    gen.markRegDefined(dest);
    //   *((int*)insn) = imm;
    // insn += sizeof(int);
  }

}}}
