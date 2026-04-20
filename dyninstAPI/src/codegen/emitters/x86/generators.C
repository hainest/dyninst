#include "arch-regs-x86.h"
#include "codegen/emitters/x86/generators.h"
#include "inst-x86.h"
#include "registerSpace/registerSpace.h"
#include "unaligned_memory_access.h"

#include <cassert>
#include <cstdint>
#include <limits>

namespace Dyninst { namespace DyninstAPI { namespace x86 {

  void emitAddMem(Address addr, int imm, codeGen &gen) {
    // This add needs to encode "special" due to an exception
    //  to the normal encoding rules and issues caused by AMD64's
    //  pc-relative data addressing mode.  Our helper functions will
    //  not correctly emit what we want, and we want this very specific
    //  mode for the add instruction.  So I'm just writing raw bytes.

    GET_PTR(insn, gen);
    if(imm < 128 && imm > -127) {
      if(gen.rs()->getAddressWidth() == 8) {
        append_memory_as_byte(insn, 0x48); // REX byte for a quad-add
      }
      append_memory_as_byte(insn, 0x83);
      append_memory_as_byte(insn, 0x04);
      append_memory_as_byte(insn, 0x25);

      assert(addr <= std::numeric_limits<uint32_t>::max() && "addr more than 32-bits");
      append_memory_as(insn, static_cast<uint32_t>(addr)); // Write address

      append_memory_as(insn, static_cast<int8_t>(imm));
      SET_PTR(insn, gen);
      return;
    }

    if(imm == 1) {
      if(gen.rs()->getAddressWidth() == 4) {
        append_memory_as_byte(insn, 0xFF); // incl
        append_memory_as_byte(insn, 0x05);
      } else {
        assert(gen.rs()->getAddressWidth() == 8);
        append_memory_as_byte(insn, 0xFF); // inlc with SIB
        append_memory_as_byte(insn, 0x04);
        append_memory_as_byte(insn, 0x25);
      }
    } else {
      append_memory_as_byte(insn, 0x81); // addl
      append_memory_as_byte(insn, 0x4);
      append_memory_as_byte(insn, 0x25);
    }

    assert(addr <= std::numeric_limits<uint32_t>::max() && "addr more than 32-bits");
    append_memory_as(insn, static_cast<uint32_t>(addr)); // Write address

    if(imm != 1) {
      append_memory_as(insn, int32_t{imm}); // Write immediate value to add
    }

    SET_PTR(insn, gen);
  }

  /*
     Emit the ModRM byte and displacement for addressing modes.
     base is a register (EAX, ECX, REGNUM_EDX, EBX, EBP, REGNUM_ESI, REGNUM_EDI)
     disp is a displacement
     reg_opcode is either a register or an opcode
  */
  void emitAddressingMode(unsigned base, RegValue disp, unsigned reg_opcode, codeGen &gen) {
    // MT linux uses ESP+4
    // we need an SIB in that case
    if(base == REGNUM_ESP) {
      emitAddressingMode(REGNUM_ESP, Null_Register, 0, disp, reg_opcode, gen);
      return;
    }
    GET_PTR(insn, gen);
    if(base == Null_Register) {
      append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, 5));
      assert(std::numeric_limits<int32_t>::lowest() <= disp &&
             disp <= std::numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
    } else if(disp == 0 && base != REGNUM_EBP) {
      append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, base));
    } else if(disp >= -128 && disp <= 127) {
      append_memory_as_byte(insn, makeModRMbyte(1, reg_opcode, base));
      append_memory_as(insn, static_cast<int8_t>(disp));
    } else {
      append_memory_as_byte(insn, makeModRMbyte(2, reg_opcode, base));
      assert(std::numeric_limits<int32_t>::lowest() <= disp &&
             disp <= std::numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
    }
    SET_PTR(insn, gen);
  }

  // VG(7/30/02): emit a fully fledged addressing mode: base+index<<scale+disp
  void emitAddressingMode(unsigned base, unsigned index, unsigned int scale, RegValue disp,
                          int reg_opcode, codeGen &gen) {
    bool needSIB = (base == REGNUM_ESP) || (index != Null_Register);

    if(!needSIB) {
      emitAddressingMode(base, disp, reg_opcode, gen);
      return;
    }

    // This isn't true for AMD-64...
    // assert(index != REGNUM_ESP);

    if(index == Null_Register) {
      assert(base == REGNUM_ESP); // not necessary, but sane
      index = 4;                  // (==REGNUM_ESP) which actually means no index in SIB
    }

    GET_PTR(insn, gen);

    if(base == Null_Register) { // we have to emit [index<<scale+disp32]
      append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, 4));
      append_memory_as_byte(insn, makeSIBbyte(scale, index, 5));
      assert(std::numeric_limits<int32_t>::lowest() <= disp &&
             disp <= std::numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
    } else if(disp == 0 && base != REGNUM_EBP) { // EBP must have 0 disp8; emit [base+index<<scale]
      append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, 4));
      append_memory_as_byte(insn, makeSIBbyte(scale, index, base));
    } else if(disp >= -128 && disp <= 127) { // emit [base+index<<scale+disp8]
      append_memory_as_byte(insn, makeModRMbyte(1, reg_opcode, 4));
      append_memory_as_byte(insn, makeSIBbyte(scale, index, base));
      append_memory_as(insn, static_cast<int8_t>(disp));
    } else { // emit [base+index<<scale+disp32]
      append_memory_as_byte(insn, makeModRMbyte(2, reg_opcode, 4));
      append_memory_as_byte(insn, makeSIBbyte(scale, index, base));
      assert(std::numeric_limits<int32_t>::lowest() <= disp &&
             disp <= std::numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
    }

    SET_PTR(insn, gen);
  }

  void emitCallRel32(unsigned disp32, codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xE8);
    append_memory_as(insn, uint32_t{disp32});
    SET_PTR(insn, gen);
  }

  void emitSegPrefix(Register segReg, codeGen &gen) {
    switch(segReg) {
      case REGNUM_FS:
        emitSimpleInsn(PREFIX_SEGFS, gen);
        return;
      case REGNUM_GS:
        emitSimpleInsn(PREFIX_SEGGS, gen);
        return;
      default:
        assert(0 && "Segment register not handled");
        return;
    }
  }

  void emitSimpleInsn(unsigned op, codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as(insn, static_cast<uint8_t>(op));
    SET_PTR(insn, gen);
  }

}}}
