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
#include "x86/opcode_xlat.h"
#include "x86/register_xlat.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "syscalls.h"
#include "x86/decoder.h"

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

  bool is_cft(di::Instruction const* insn) { return insn->isBranch() || insn->isCall(); }

}

namespace Dyninst { namespace InstructionAPI {

  x86_decoder::x86_decoder(Dyninst::Architecture a) : InstructionDecoderImpl(a) {

    mode = (a == Dyninst::Arch_x86_64) ? CS_MODE_64 : CS_MODE_32;

    auto create = [this](disassem& d, cs_opt_value v) {
      cs_open(CS_ARCH_X86, this->mode, &d.handle);
      cs_option(d.handle, CS_OPT_DETAIL, v);
      d.insn = cs_malloc(d.handle);
    };

    /*
     *  With details enabled, a Capstone instruction object has complete information.
     *
     *  This is used in 'decodeOperands' because all of the details are needed.
     */
    create(dis_with_detail, CS_OPT_ON);

    /*
     *  Without details, a Capstone instruction object has fewer populated fields
     *  (e.g., no operand details) so takes up less space and time. Capstone instruction
     *  objects _always_ populate the mnemonic and a string representation of the operands.
     *
     *  This is used in 'decodeOpcode' to quickly create an Instruction object.
     */
    create(dis_without_detail, CS_OPT_OFF);
  }

  x86_decoder::~x86_decoder() {
    cs_free(dis_with_detail.insn, 1);
    cs_close(&dis_with_detail.handle);

    cs_free(dis_without_detail.insn, 1);
    cs_close(&dis_without_detail.handle);
  }

  void x86_decoder::decodeOpcode(InstructionDecoder::buffer& buf) {
    auto* code = buf.start;
    size_t codeSize = buf.end - buf.start;
    uint64_t cap_addr = 0;

    // We want this to be as fast as possible, so don't have Capstone provide all details.
    auto& dis = dis_without_detail;

    // The iterator form of disassembly allows reuse of the instruction object, reducing
    // the number of memory allocations.
    if(!cs_disasm_iter(dis.handle, &code, &codeSize, &cap_addr, dis.insn)) {
      // Gap parsing can trigger this case. In particular, when it encounters prefixes in an invalid
      // order. Notably, if a REX prefix (0x40-0x48) appears followed by another prefix (0x66, 0x67,
      // etc) we'll reject the instruction as invalid and send it back with no entry.  Since this is
      // a common byte sequence to see in, for example, ASCII strings, we want to simply accept this
      // and move on.
      decode_printf("Failed to disassemble instruction at %p: %s\n", code,
                    cs_strerror(cs_errno(dis.handle)));
      m_Operation = Operation(e_No_Entry, "INVALID", m_Arch);
      return;
    }

    entryID e = x86::translate_opcode(static_cast<x86_insn>(dis.insn->id));
    m_Operation = Operation(e, dis.insn->mnemonic, m_Arch);
    buf.start += dis.insn->size;
  }

  void x86_decoder::doDelayedDecode(Instruction const* insn) {
    auto* code = static_cast<unsigned char const*>(insn->ptr());
    size_t codeSize = insn->size();
    uint64_t cap_addr = 0;

    // We need all of the instruction details in order to unpack the operands
    auto& dis = dis_with_detail;

    // The iterator form of disassembly allows reuse of the instruction object, reducing
    // the number of memory allocations.
    if(!cs_disasm_iter(dis.handle, &code, &codeSize, &cap_addr, dis.insn)) {
      decode_printf("Failed to disassemble instruction at %p: %s\n", code,
                    cs_strerror(cs_errno(dis.handle)));
      m_Operation = Operation(e_No_Entry, "INVALID", m_Arch);
      return;
    }
    decode_operands(insn, dis);
  }

  bool x86_decoder::decodeOperands(Instruction const* ins) {
    // We need the full-detail decode to get the operands
    if(!dis_with_detail.insn) {
      this->doDelayedDecode(ins);
    }
    return true;
  }

  Instruction x86_decoder::decode(InstructionDecoder::buffer& b) {
    const unsigned char* start = b.start;
    decodeOpcode(b);
    unsigned int decodedSize = b.start - start;
    return Instruction(m_Operation, decodedSize, start, m_Arch);
  }

  void x86_decoder::decode_operands(Instruction const* insn, disassem dis) {
    // Categories must be decoded before anything else since they are used
    // in the other decoding steps.
    insn->categories = x86::decode_categories(insn, dis);

    /* Decode _explicit_ operands
     *
     * There are three types:
     *
     *   add r1, r2       ; r1, r2 are both X86_OP_REG
     *   jmp -64          ; -64 is X86_OP_IMM
     *   mov r1, [0x33]   ; r1 is X86_OP_REG, 0x33 is X86_OP_MEM
     */
    auto* d = dis.insn->detail;
    for(uint8_t i = 0; i < d->x86.op_count; ++i) {
      cs_x86_op const& operand = d->x86.operands[i];
      switch(operand.type) {
        case X86_OP_REG:
          decode_reg(insn, operand);
          break;
        case X86_OP_IMM:
          decode_imm(insn, operand, dis);
          break;
        case X86_OP_MEM:
          break;
        case X86_OP_INVALID:
          decode_printf("[0x%lx %s %s] has an invalid operand.\n", dis.insn->address,
                        dis.insn->mnemonic, dis.insn->op_str);
          break;
      }
    }
  }

  void x86_decoder::decode_reg(Instruction const* insn, cs_x86_op const& operand) {
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

  void x86_decoder::decode_imm(Instruction const* insn, cs_x86_op const& operand, disassem dis) {
    auto const size = dis.insn->detail->x86.encoding.imm_size;
    auto const type = size_to_type_signed(size);
    auto imm = Immediate::makeImmediate(Result(type, operand.imm));

    if(!is_cft(insn)) {
      insn->appendOperand(std::move(imm), false, false, false);
      return;
    }

    auto IP(makeRegisterExpression(MachRegister::getPC(m_Arch)));

    auto const isCall = insn->isCall();
    auto const isConditional = insn->isConditional();
    auto const usesRelativeAddressing = cs_insn_group(dis.handle, dis.insn, CS_GRP_BRANCH_RELATIVE);

    if(usesRelativeAddressing) {
      // Capstone adjusts the offset to account for the current instruction's length, so we can
      // just create an addition AST expression here.
      auto target(makeAddExpression(IP, imm, s64));
      insn->addSuccessor(std::move(target), isCall, false, isConditional, false);
    } else {
      insn->addSuccessor(std::move(imm), isCall, false, isConditional, false);
    }
    if(isConditional) {
      insn->addSuccessor(std::move(IP), false, false, true, true);
    }
  }

}}
