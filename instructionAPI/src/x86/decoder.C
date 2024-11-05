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

#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "categories.h"
#include "debug.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "syscalls.h"
#include "type_conversion.h"
#include "x86/decoder.h"
#include "x86/opcode_xlat.h"
#include "x86/register_xlat.h"

#include <algorithm>
#include <cstdint>

/***************************************************************************
 * The work here is based on
 *
 *  Intel 64 and IA-32 Architectures Software Developer’s Manual (SDM)
 *  December 2021
 *
 *  Intel Architecture Instruction Set Extensions and Future Features (IISE)
 *  May 2021
 *
 *  AMD64 Architecture Programmer’s Manual (AMDAPM)
 *  Revision 3.33
 *  November 2021
 *
 ***************************************************************************/

namespace di = Dyninst::InstructionAPI;

namespace {

  bool is_cft(di::Instruction const *insn) {
    return insn->isBranch() || insn->isCall();
  }

}

namespace Dyninst { namespace InstructionAPI {

    x86_decoder::x86_decoder(Dyninst::Architecture a) : InstructionDecoderImpl(a) {

      mode = (a == Dyninst::Arch_x86_64) ? CS_MODE_64 : CS_MODE_32;

      cs_open(CS_ARCH_X86, this->mode, &disassembler.handle);
      cs_option(disassembler.handle, CS_OPT_DETAIL, CS_OPT_ON);
      disassembler.insn = cs_malloc(disassembler.handle);
    }

    x86_decoder::~x86_decoder() {
      cs_free(disassembler.insn, 1);
      cs_close(&disassembler.handle);
    }

    Instruction x86_decoder::decode(InstructionDecoder::buffer &buf) {
      auto *code = buf.start;
      size_t codeSize = buf.end - buf.start;
      uint64_t cap_addr = 0;

      // The iterator form of disassembly allows reuse of the instruction object, reducing
      // the number of memory allocations.
      if(!cs_disasm_iter(disassembler.handle, &code, &codeSize, &cap_addr, disassembler.insn)) {
        // Gap parsing can trigger this case. In particular, when it encounters prefixes in an invalid
        // order. Notably, if a REX prefix (0x40-0x48) appears followed by another prefix (0x66, 0x67,
        // etc) we'll reject the instruction as invalid and send it back with no entry.  Since this is
        // a common byte sequence to see in, for example, ASCII strings, we want to simply accept this
        // and move on.
        decode_printf("Failed to disassemble instruction at %p: %s\n", code,
                      cs_strerror(cs_errno(disassembler.handle)));
        m_Operation = Operation(e_No_Entry, "INVALID", m_Arch);
        return {};
      }

      entryID e = x86::translate_opcode(static_cast<x86_insn>(disassembler.insn->id));
      m_Operation = Operation(e, disassembler.insn->mnemonic, m_Arch);
      buf.start += disassembler.insn->size;
      unsigned int decodedSize = buf.start - code;
      Instruction insn(m_Operation, decodedSize, code, m_Arch);
      decode_operands(&insn);
      return insn;
    }

    void x86_decoder::decode_operands(Instruction const *insn) {
      // Categories must be decoded before anything else since they are used
      // in the other decoding steps.
      insn->categories = x86::decode_categories(insn, disassembler);

      /* Decode _explicit_ operands
       *
       * There are three types:
       *
       *   add r1, r2       ; r1, r2 are both X86_OP_REG
       *   jmp -64          ; -64 is X86_OP_IMM
       *   mov r1, [0x33]   ; r1 is X86_OP_REG, 0x33 is X86_OP_MEM
       */
      auto *d = disassembler.insn->detail;
      for(uint8_t i = 0; i < d->x86.op_count; ++i) {
        cs_x86_op const &operand = d->x86.operands[i];
        switch(operand.type) {
          case X86_OP_REG:
            decode_reg(insn, operand);
            break;
          case X86_OP_IMM:
            decode_imm(insn, operand);
            break;
          case X86_OP_MEM:
            decode_mem(insn, operand);
            break;
          case X86_OP_INVALID:
            decode_printf("[0x%lx %s %s] has an invalid operand.\n", disassembler.insn->address,
                          disassembler.insn->mnemonic, disassembler.insn->op_str);
            break;
        }
      }
    }

    void x86_decoder::decode_reg(Instruction const *insn, cs_x86_op const &operand) {
      auto regAST = makeRegisterExpression(x86::translate_register(operand.reg, mode));

      const bool isCall = insn->isCall();
      if(insn->isBranch() || isCall) {
        insn->addSuccessor(regAST, isCall, true, false, false);
        return;
      }

      // It's an error if an operand is neither read nor written.
      // In this case, we mark it as both read and written to be conservative.
      bool isRead = ((operand.access & CS_AC_READ) != 0);
      bool isWritten = ((operand.access & CS_AC_WRITE) != 0);
      if(!isRead && !isWritten) {
        isRead = isWritten = true;
      }
      insn->appendOperand(regAST, isRead, isWritten, false);
    }

    void x86_decoder::decode_imm(Instruction const *insn, cs_x86_op const &operand) {
      auto const type = size_to_type_signed(operand.size);
      auto imm = Immediate::makeImmediate(Result(type, operand.imm));

      constexpr bool isRead = true;
      constexpr bool isWritten = true;
      constexpr bool isImplicit = true;
      constexpr bool isIndirect = true;

      if(!is_cft(insn)) {
        insn->appendOperand(std::move(imm), !isRead, !isWritten, !isImplicit);
        return;
      }

      auto IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));

      auto const isCall = insn->isCall();
      auto const isConditional = insn->isConditional();
      auto const usesRelativeAddressing = cs_insn_group(disassembler.handle, disassembler.insn, CS_GRP_BRANCH_RELATIVE);
      bool const isFallthrough = insn->allowsFallThrough();

      if(usesRelativeAddressing) {
        insn->appendOperand(imm, isRead, !isWritten, !isImplicit);
        insn->appendOperand(IP, isRead, isWritten, isImplicit);
        // Capstone adjusts the offset to account for the current instruction's length, so we can
        // just create an addition AST expression here.
        auto target(makeAddExpression(IP, imm, s64));
        insn->addSuccessor(std::move(target), isCall, !isIndirect, isConditional, isFallthrough);
      } else {
        insn->addSuccessor(std::move(imm), isCall, !isIndirect, isConditional, isFallthrough);
      }
      if(isConditional) {
        constexpr bool is_call = true;
        constexpr bool is_conditional = true;
        insn->addSuccessor(std::move(IP), !is_call, !isIndirect, is_conditional, isFallthrough);
      }
    }

    void x86_decoder::decode_mem(Instruction const *insn, cs_x86_op const &operand) {
      /*
       *  SDM 3.7.5 Specifying an Offset
       *
       *  The offset part of a memory address can be specified directly as a static value (called
       *  a displacement) or through an address computation made up of one or more of the following
       *  components:
       *
       *  - Displacement: An 8-, 16-, or 32-bit value.
       *  - Base:         The value in a general-purpose register.
       *  - Index:        The value in a general-purpose register.
       *  - Scale factor: A value of 2, 4, or 8 that is multiplied by the index value.
       *
       *  Offset = Base + (Index * Scale) + Displacement
       *
       *  The offset which results from adding these components is called an effective address. Each
       *  of these components can have either a positive or negative (2s complement) value, with the
       *  exception of the scaling factor.
       */

      auto disp = [&]() -> Expression::Ptr {
        // Capstone: Displacement value, valid if encoding.disp_offset != 0
        if(disassembler.insn->detail->x86.encoding.disp_offset == 0) {
          return {};
        }
        auto const size = disassembler.insn->detail->x86.encoding.disp_size;
        auto const type = size_to_type_signed(size);
        return Immediate::makeImmediate(Result(type, operand.mem.disp));
      }();

      auto base = [&]() -> Expression::Ptr {
        if(operand.mem.base == X86_REG_INVALID) {
          return {};
        }
        auto const basereg = x86::translate_register(operand.mem.base, this->mode);
        return makeRegisterExpression(basereg);
      }();

      auto index = [&]() -> Expression::Ptr {
        if(operand.mem.index == X86_REG_INVALID) {
          return {};
        }
        if(operand.mem.index == X86_REG_EIZ || operand.mem.index == X86_REG_RIZ) {
          // {E,R}IZ is a pseudo register used when the index register is explicitly
          // set to zero in the opcode. It's used by decoders, but has no meaning
          // for the ISA.
          return {};
        }
        auto reg = x86::translate_register(operand.mem.index, this->mode);
        return makeRegisterExpression(reg);
      }();

      auto scaled_index = [&]() -> Expression::Ptr {
        // SDM: A scale factor may be used only when an index also is used
        if(!index) {
          return {};
        }
        auto scale = Immediate::makeImmediate(Result(u8, operand.mem.scale));

        // The size of the Index register *should* be the same as the size
        // of the operand, but I can't find a requirement for that in the
        // SDM so don't assume any sizes here.
        auto reg = x86::translate_register(operand.mem.index, this->mode);
        auto const type = size_to_type_signed(reg.size());
        return makeMultiplyExpression(std::move(index), std::move(scale), type);
      }();

      // Calculate `(Index * Scale) + Displacement`
      auto rhs = [&]() -> Expression::Ptr {
        if(!scaled_index) {
          return disp;
        }
        if(!disp) {
          return scaled_index;
        }

        // As with scaled_index, this shouldn't be necessary but be careful.
        auto const index_reg = x86::translate_register(operand.mem.index, this->mode);
        auto const disp_size = disassembler.insn->detail->x86.encoding.disp_size;
        auto const size = std::max(index_reg.size(), static_cast<std::uint32_t>(disp_size));
        auto const type = size_to_type_signed(size);
        return makeAddExpression(std::move(scaled_index), std::move(disp), type);
      }();

      auto effectiveAddr = [&]() -> Expression::Ptr {
        if(!rhs) {
          return base;
        }
        if(!base) {
          return rhs;
        }
        auto const type = size_to_type_signed(operand.size);
        return makeAddExpression(std::move(base), std::move(rhs), type);
      }();

      auto finalAddress = [&]() -> Expression::Ptr {
        if(operand.mem.segment == X86_REG_INVALID) {
          return effectiveAddr;
        }

        // Real-Address Mode `Seg + effectiveAddr`
        auto const reg = x86::translate_register(operand.mem.segment, this->mode);
        auto segReg = makeRegisterExpression(reg);

        // It's an absolute address, so it's unsigned
        auto const type = size_to_type_unsigned(operand.size);
        return makeAddExpression(std::move(segReg), std::move(effectiveAddr), type);
      }();

      // Capstone may report register operands as neither read nor written.
      // In this case, we mark it as both read and written to be conservative.
      bool isRead = ((operand.access & CS_AC_READ) != 0);
      bool isWritten = ((operand.access & CS_AC_WRITE) != 0);
      if(!isRead && !isWritten) {
        isRead = isWritten = true;
      }

      // LEA (Load Effective Address) does not dereference its memory operand
      if(insn->getOperation().getID() == e_lea) {
        insn->appendOperand(std::move(finalAddress), isRead, isWritten, false);
        return;
      }

      // has a control flow target (e.g., `call [rbx + rax * 2 + 0xff]`)
      if(is_cft(insn)) {
        auto const is_call = insn->isCall();
        auto const is_fallthrough = insn->allowsFallThrough();
        auto const type = size_to_type_signed(operand.size);
        auto expr = makeDereferenceExpression(std::move(finalAddress), type);
        insn->addSuccessor(std::move(expr), is_call, true, false, is_fallthrough, false);
        return;
      }

      // x87 instruction
      if(cs_insn_group(disassembler.handle, disassembler.insn, X86_GRP_FPU)) {
        auto const type = size_to_type_float(operand.size);
        auto expr = makeDereferenceExpression(std::move(finalAddress), type);
        insn->appendOperand(std::move(expr), isRead, isWritten, false);
        return;
      }

      // vector instruction
      if(insn->isVector()) {
        auto const type = size_to_type_memory(operand.size);
        auto expr = makeDereferenceExpression(std::move(finalAddress), type);
        insn->appendOperand(std::move(expr), isRead, isWritten, false);
        return;
      }

      // basic instruction (e.g., `mov rax, [rbx + rax * 2 + 0xff]`)
      auto const type = size_to_type_signed(operand.size);
      auto expr = makeDereferenceExpression(std::move(finalAddress), type);
      insn->appendOperand(std::move(expr), isRead, isWritten, false);
    }

}}
