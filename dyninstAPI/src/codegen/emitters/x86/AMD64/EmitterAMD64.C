#include "ABI.h"
#include "addressSpace.h"
#include "arch-regs-x86.h"
#include "arch-x86.h"
#include "binaryEdit.h"
#include "BPatch.h"
#include "codegen/emitters/x86/AMD64/EmitterAMD64.h"
#include "codegen/emitters/x86/AMD64/generators.h"
#include "codegen/RegControl.h"
#include "debug.h"
#include "function.h"
#include "image.h"
#include "inst-x86.h"
#include "liveness.h"
#include "parse_func.h"
#include "RegisterConversion.h"
#include "registerSpace/RealRegister.h"
#include "registerSpace/registerSpace.h"
#include "Symbol.h"
#include "unaligned_memory_access.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <limits>

extern bool isPowerOf2(int value, int &result);

static int extra_space_check{};

// this is the distance on the basetramp stack frame from the
// start of the GPR save region to where the base pointer is,
// in 8-byte quadwords
#define GPR_SAVE_REGION_OFFSET 18

static constexpr auto AMD64_RED_ZONE = 0x80;

static Register amd64_arg_regs[] = {REGNUM_RDI, REGNUM_RSI, REGNUM_RDX,
                                    REGNUM_RCX, REGNUM_R8,  REGNUM_R9};
#define AMD64_ARG_REGS (sizeof(amd64_arg_regs) / sizeof(Register))

// this is the distance in 8-byte quadwords from the frame pointer
// in our basetramp's stack frame to the saved value of RFLAGS
// (1 qword for our false return address, 16 for the saved registers, 1 more for the flags)
#define SAVED_RFLAGS_OFFSET 18

namespace Dyninst { namespace DyninstAPI {

  bool EmitterAMD64::emitCallRelative(Register, Address, Register, codeGen &) {
    assert(0);
    return false;
  }

  bool EmitterAMD64::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
    AMD64::emitMovRegToReg64(dest, src, true, gen);
    gen.markRegDefined(dest);
    return true;
  }

  bool EmitterAMD64::emitMoveRegToReg(registerSlot *source, registerSlot *dest, codeGen &gen) {
    // TODO: make this work for getting the flag register too.

    return emitMoveRegToReg(source->encoding(), dest->encoding(), gen);
  }

  void EmitterAMD64::emitLEA(Register base, Register index, unsigned int scale, int disp,
                             Register dest, codeGen &gen) {
    Register tmp_base = base;
    Register tmp_index = index;
    Register tmp_dest = dest;
    AMD64::emitRex(/*is_64*/ true, &tmp_dest, tmp_index == Null_Register ? NULL : &tmp_index,
                   tmp_base == Null_Register ? NULL : &tmp_base, gen);
    ::emitLEA(RealRegister(tmp_base), RealRegister(tmp_index), scale, disp, RealRegister(tmp_dest),
              gen);
    gen.markRegDefined(dest);
  }

  codeBufIndex_t EmitterAMD64::emitIf(Register expr_reg, Register target, RegControl,
                                      codeGen &gen) {
    // test %expr, %expr
    AMD64::emitOpRegReg64(0x85, expr_reg, expr_reg, true, gen);

    // Retval: where the jump is in this sequence
    codeBufIndex_t retval = gen.getIndex();

    // Jump displacements are from the end of the insn, not start. The
    // one we're emitting has a size of 6.
    int32_t disp = target - 6;

    // je target
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x0F);
    append_memory_as_byte(insn, 0x84);
    append_memory_as(insn, int32_t{disp});
    SET_PTR(insn, gen);

    return retval;
  }

  void EmitterAMD64::emitOp(unsigned opcode, Register dest, Register src1, Register src2,
                            codeGen &gen) {
    // TODO: optimize this further for ops where order doesn't matter
    if(src1 != dest) {
      AMD64::emitMovRegToReg64(dest, src1, true, gen);
    }
    AMD64::emitOpRegReg64(opcode, dest, src2, true, gen);
    gen.markRegDefined(dest);
  }

  void EmitterAMD64::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1,
                               RegValue src2imm, codeGen &gen) {
    if(src1 != dest) {
      AMD64::emitMovRegToReg64(dest, src1, true, gen);
    }
    AMD64::emitOpRegImm64(opcode1, opcode2, dest, src2imm, true, gen);
    gen.markRegDefined(dest);
  }

  void EmitterAMD64::emitRelOp(unsigned op, Register dest, Register src1, Register src2,
                               codeGen &gen, bool s) {
    // cmp %src2, %src1
    AMD64::emitOpRegReg64(0x39, src2, src1, true, gen);

    // mov $0, $dest ; done now in case src1 == dest or src2 == dest
    // (we can do this since mov doesn't mess w/ flags)
    AMD64::emitMovImmToReg64(dest, 0, false, gen);
    gen.markRegDefined(dest);

    // jcc by two or three, depdending on size of mov
    unsigned char jcc_opcode = jccOpcodeFromRelOp(op, s);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, jcc_opcode);
    SET_PTR(insn, gen);

    codeBufIndex_t jcc_disp = gen.used();
    gen.fill(1, codeGen::cgNOP);
    codeBufIndex_t after_jcc = gen.used();
    // mov $2,  %dest

    AMD64::emitMovImmToReg64(dest, 1, false, gen);
    codeBufIndex_t after_mov = gen.used();

    gen.setIndex(jcc_disp);
    REGET_PTR(insn, gen);
    append_memory_as_byte(insn, codeGen::getDisplacement(after_jcc, after_mov));
    SET_PTR(insn, gen);

    gen.setIndex(after_mov); // overrides previous SET_PTR
  }

  void EmitterAMD64::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
                                  codeGen &gen, bool s) {
    // cmp $src2imm, %src1
    AMD64::emitOpRegImm64(0x81, 7, src1, src2imm, true, gen);

    // mov $0, $dest ; done now in case src1 == dest
    // (we can do this since mov doesn't mess w/ flags)
    AMD64::emitMovImmToReg64(dest, 0, false, gen);
    gen.markRegDefined(dest);

    // jcc by two or three, depdending on size of mov
    unsigned char opcode = jccOpcodeFromRelOp(op, s);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    SET_PTR(insn, gen);
    codeBufIndex_t jcc_disp = gen.used();
    gen.fill(1, codeGen::cgNOP);
    codeBufIndex_t after_jcc = gen.used();

    // mov $2,  %dest
    AMD64::emitMovImmToReg64(dest, 1, false, gen);
    codeBufIndex_t after_mov = gen.used();

    gen.setIndex(jcc_disp);
    REGET_PTR(insn, gen);
    append_memory_as_byte(insn, codeGen::getDisplacement(after_jcc, after_mov));
    SET_PTR(insn, gen);

    gen.setIndex(after_mov); // overrides previous SET_PTR
  }

  void EmitterAMD64::emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s) {
    // TODO: fix so that we don't always use RAX

    // push RDX if it's in use, since we will need it
    bool save_rdx = false;
    if(!gen.rs()->isFreeRegister(REGNUM_RDX) && (dest != REGNUM_RDX)) {
      save_rdx = true;
      AMD64::emitPushReg64(REGNUM_RDX, gen);
    } else {
      gen.markRegDefined(REGNUM_RDX);
    }

    // If src2 is RDX we need to move it into a scratch register, as the sign extend
    // will overwrite RDX.
    // Note that this does not imply RDX is not free; both inputs are free if they
    // are not used after this call.
    Register scratchReg = src2;
    if(scratchReg == REGNUM_RDX) {
      std::vector<Register> dontUse;
      dontUse.push_back(REGNUM_RAX);
      dontUse.push_back(src2);
      dontUse.push_back(dest);
      dontUse.push_back(src1);
      scratchReg = gen.rs()->getScratchRegister(gen, dontUse);
      AMD64::emitMovRegToReg64(scratchReg, src2, true, gen);
    }
    gen.markRegDefined(scratchReg);

    // mov %src1, %rax
    AMD64::emitMovRegToReg64(REGNUM_RAX, src1, true, gen);
    gen.markRegDefined(REGNUM_RAX);

    // cqo (sign extend RAX into RDX)
    emitSimpleInsn(0x48, gen); // REX.W
    emitSimpleInsn(0x99, gen);

    if(s) {
      // idiv %src2
      AMD64::emitOpRegReg64(0xF7, 0x7, scratchReg, true, gen);
    } else {
      // div %src2
      AMD64::emitOpRegReg64(0xF7, 0x6, scratchReg, true, gen);
    }

    // mov %rax, %dest
    AMD64::emitMovRegToReg64(dest, REGNUM_RAX, true, gen);
    gen.markRegDefined(dest);

    // pop rdx if it needed to be saved
    if(save_rdx) {
      AMD64::emitPopReg64(REGNUM_RDX, gen);
    }
  }

  void EmitterAMD64::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen) {
    int result = -1;

    gen.markRegDefined(dest);
    if(isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      // immediate is a power of two - use a shift
      // mov %src1, %dest (if needed)
      if(src1 != dest) {
        AMD64::emitMovRegToReg64(dest, src1, true, gen);
      }
      // sal dest, result
      // Note: sal and shl are the same
      AMD64::emitOpRegImm8_64(0xC1, 4, dest, result, true, gen);
    } else {
      // imul %dest, %src1, $src2imm
      AMD64::emitOpRegRegImm64(0x69, dest, src1, src2imm, true, gen);
    }
  }

  void EmitterAMD64::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen,
                                bool s) {
    int result = -1;
    gen.markRegDefined(dest);
    if(isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      // divisor is a power of two - use a shift instruction
      // mov %src1, %dest (if needed)
      if(src1 != dest) {
        AMD64::emitMovRegToReg64(dest, src1, true, gen);
      }
      if(s) {
        // sar $result, %dest
        AMD64::emitOpRegImm8_64(0xC1, 7, dest, result, true, gen);
      } else {
        // shr $result, %dest
        AMD64::emitOpRegImm8_64(0xC1, 5, dest, result, true, gen);
      }
    } else {

      // push RDX if it's in use, since we will need it
      bool save_rdx = false;
      if(!gen.rs()->isFreeRegister(REGNUM_RDX) && (dest != REGNUM_RDX)) {
        save_rdx = true;
        AMD64::emitPushReg64(REGNUM_RDX, gen);
      } else {
        gen.markRegDefined(REGNUM_RDX);
      }
      // need to put dividend in RDX:RAX
      // mov %src1, %rax
      AMD64::emitMovRegToReg64(REGNUM_EAX, src1, true, gen);
      gen.markRegDefined(REGNUM_RAX);
      // We either do a sign extension from RAX to RDX or clear RDX
      if(s) {
        emitSimpleInsn(0x48, gen); // REX.W
        emitSimpleInsn(0x99, gen);
      } else {
        AMD64::emitMovImmToReg64(REGNUM_RDX, 0, true, gen);
      }
      // push immediate operand on the stack (no IDIV $imm)
      emitPushImm(src2imm, gen);

      if(s) {
        // idiv (%rsp)
        AMD64::emitOpRegRM64(0xF7, 0x7 /* opcode extension */, REGNUM_RSP, 0, true, gen);
      } else {
        // div (%rsp)
        AMD64::emitOpRegRM64(0xF7, 0x6 /* opcode extension */, REGNUM_RSP, 0, true, gen);
      }

      // mov %rax, %dest ; set the result
      AMD64::emitMovRegToReg64(dest, REGNUM_RAX, true, gen);

      // pop the immediate off the stack
      // add $8, %rsp
      AMD64::emitOpRegImm8_64(0x83, 0x0, REGNUM_RSP, 8, true, gen);
      gen.rs()->incStack(-8);

      // pop rdx if it needed to be saved
      if(save_rdx) {
        AMD64::emitPopReg64(REGNUM_RDX, gen);
      }
    }
  }

  void EmitterAMD64::emitLoad(Register dest, Address addr, int size, codeGen &gen) {

    Register scratch = gen.rs()->getScratchRegister(gen);

    // mov $addr, %rax
    AMD64::emitMovImmToReg64(scratch, addr, true, gen);

    // mov (%rax), %dest
    AMD64::emitMovRMToReg64(dest, scratch, 0, size, gen);
    gen.rs()->freeRegister(scratch);
    gen.markRegDefined(dest);
  }

  void EmitterAMD64::emitLoadConst(Register dest, Address imm, codeGen &gen) {
    AMD64::emitMovImmToReg64(dest, imm, true, gen);
    gen.markRegDefined(dest);
  }

  void EmitterAMD64::emitLoadIndir(Register dest, Register addr_src, int size, codeGen &gen) {
    AMD64::emitMovRMToReg64(dest, addr_src, 0, size, gen);
    gen.markRegDefined(dest);
  }

  void EmitterAMD64::emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen) {
    if(gen.bt()->createdFrame) {
      Register scratch = gen.rs()->getScratchRegister(gen);
      // mov (%rbp), %rax
      AMD64::emitMovRMToReg64(scratch, REGNUM_RBP, 0, 8, gen);

      // mov offset(%rax), %dest
      AMD64::emitMovRMToReg64(dest, scratch, offset, 4, gen);
      return;
    }
    AMD64::emitMovRMToReg64(dest, REGNUM_RBP, offset, 4, gen);
  }

  bool EmitterAMD64::emitLoadRelative(Register dest, Address offset, Register base, int /* size */,
                                      codeGen &gen) {
    // mov offset(%base), %dest
    AMD64::emitMovRMToReg64(dest, base, offset, gen.addrSpace()->getAddressWidth(), gen);
    gen.markRegDefined(dest);
    return true;
  }

  bool EmitterAMD64::emitLoadRelativeSegReg(Register dest, Address offset, Register base,
                                            int /* size */, codeGen &gen) {
    AMD64::emitMovSegRMToReg64(dest, base, offset, gen);
    gen.markRegDefined(dest);
    return true;
  }

  void EmitterAMD64::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen) {
    // mov (%rbp), %dest
    if(gen.bt()->createdFrame) {
      AMD64::emitMovRMToReg64(dest, REGNUM_RBP, 0, 8, gen);

      // add $offset, %dest
      AMD64::emitOpRegImm64(0x81, 0x0, dest, offset, 8, gen);
      gen.markRegDefined(dest);
      return;
    }
    emitLEA(REGNUM_RBP, Null_Register, 0, offset, dest, gen);
  }

  void EmitterAMD64::emitLoadOrigRegRelative(Register dest, Address offset, Register base,
                                             codeGen &gen, bool store) {
    Register scratch = gen.rs()->getScratchRegister(gen);
    gen.markRegDefined(scratch);
    gen.markRegDefined(dest);
    // either load the address or the contents at that address
    if(store) {
      // load the stored register 'base' into RAX
      emitLoadOrigRegister(base, scratch, gen);
      // move offset(%rax), %dest
      AMD64::emitMovRMToReg64(dest, scratch, offset, 4, gen);
    } else {
      // load the stored register 'base' into dest
      emitLoadOrigRegister(base, dest, gen);
      // add $offset, %dest
      AMD64::emitOpRegImm64(0x81, 0x0, dest, offset, true, gen);
    }
  }

  void EmitterAMD64::emitLoadOrigRegister(Address register_num, Register destination,
                                          codeGen &gen) {
    registerSlot *src = (*gen.rs())[register_num];
    assert(src);
    registerSlot *dest = (*gen.rs())[destination];
    assert(dest);

    if(register_num == REGNUM_ESP) {
      stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
      if(!gen.bt() || gen.bt()->alignedStack) {
        AMD64::emitMovRMToReg64(destination, loc.reg.reg(), loc.offset, 8, gen);
      } else {
        emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, destination, gen);
      }
      return;
    }

    if(src->spilledState == registerSlot::unspilled) {
      assert(register_num != REGNUM_EFLAGS);
      emitMoveRegToReg((Register)register_num, destination, gen);
      return;
    }

    stackItemLocation loc = getHeightOf(stackItem(RealRegister(register_num)), gen);
    registerSlot *stack = (*gen.rs())[loc.reg.reg()];
    emitLoadRelative(dest->encoding(), loc.offset, stack->encoding(),
                     gen.addrSpace()->getAddressWidth(), gen);
    gen.markRegDefined(destination);
    return;
  }

  void EmitterAMD64::emitStoreOrigRegister(Address register_num, Register src, codeGen &gen) {
    assert(gen.addrSpace());
    unsigned size = (gen.addrSpace()->getAddressWidth());
    gen.rs()->writeProgramRegister(gen, register_num, src, size);
  }

  void EmitterAMD64::emitStore(Address addr, Register src, int size, codeGen &gen) {
    Register scratch = gen.rs()->getScratchRegister(gen);
    gen.markRegDefined(scratch);

    // mov $addr, %rax
    AMD64::emitMovImmToReg64(scratch, addr, true, gen);

    // mov %src, (%rax)
    AMD64::emitMovRegToRM64(scratch, 0, src, size, gen);
  }

  void EmitterAMD64::emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen) {
    AMD64::emitMovRegToRM64(addr_reg, 0, src, size, gen);
  }

  void EmitterAMD64::emitStoreFrameRelative(Address offset, Register src, Register /*scratch*/,
                                            int size, codeGen &gen) {
    if(gen.bt()->createdFrame) {
      Register scratch = gen.rs()->getScratchRegister(gen);
      gen.markRegDefined(scratch);
      // mov (%rbp), %rax
      AMD64::emitMovRMToReg64(scratch, REGNUM_RBP, 0, 8, gen);
      // mov %src, offset(%rax)
      AMD64::emitMovRegToRM64(scratch, offset, src, size, gen);
      gen.rs()->freeRegister(scratch);
      return;
    }
    AMD64::emitMovRegToRM64(REGNUM_RBP, offset, src, size, gen);
  }

  void EmitterAMD64::emitStoreRelative(Register src, Address offset, Register base, int /* size */,
                                       codeGen &gen) {
    AMD64::emitMovRegToRM64(base, offset, src, gen.addrSpace()->getAddressWidth(), gen);
  }

  /* Recursive function that goes to where our instrumentation is calling
     to figure out what registers are clobbered there, and in any function
     that it calls, to a certain depth ... at which point we clobber everything

     Update-12/06, njr, since we're going to a cached system we are just going to
     look at the first level and not do recursive, since we would have to also
     store and reexamine every call out instead of doing it on the fly like before*/
  bool EmitterAMD64::clobberAllFuncCall(registerSpace *rs, func_instance *callee)

  {
    if(callee == NULL) {
      return false;
    }

    if(clobbered_functions.contains(callee)) {
      return true;
    }
    clobbered_functions.insert(callee);

    if(writesFPRs(callee->ifunc())) {
      for(unsigned i = 0; i < rs->FPRs().size(); i++) {
        // We might want this to be another flag, actually
        rs->FPRs()[i]->beenUsed = true;
      }
    }

    // Since we are making a call, mark all caller-saved registers
    // as used (therefore we will save them if they are live)
    for(int i = 0; i < rs->numGPRs(); i++) {
      rs->GPRs()[i]->beenUsed = true;
    }
    return true;
  }

  Register EmitterAMD64::emitCall(opCode op, codeGen &gen,
                                  const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands,
                                  bool noCost, func_instance *callee) {
    assert(op == callOp);
    std::vector<Register> srcs;

    bool inInstrumentation = true;

    //  Sanity check for NULL address arg
    if(!callee) {
      char msg[256];
      sprintf(msg,
              "%s[%d]:  internal error:  emitFuncCall called w/out"
              "callee argument",
              __FILE__, __LINE__);
      showErrorCallback(80, msg);
      assert(0);
    }

    // Before we generate argument code, save any register that's live across
    // the call.
    std::vector<pair<unsigned, int>> savedRegsToRestore;
    if(inInstrumentation) {
      bitArray regsClobberedByCall = ABI::getABI(8)->getCallWrittenRegisters();
      for(int i = 0; i < gen.rs()->numGPRs(); i++) {
        registerSlot *reg = gen.rs()->GPRs()[i];
        Register r = reg->encoding();
        static LivenessAnalyzer live(8);
        bool callerSave =
            regsClobberedByCall.test(live.getIndex(regToMachReg64.equal_range(r).first->second));
        if(!callerSave) {
          // We don't care!
          regalloc_printf("%s[%d]: pre-call, skipping callee-saved register %u\n", FILE__, __LINE__,
                          reg->number.getId());
          continue;
        }

        regalloc_printf(
            "%s[%d]: pre-call, register %u has refcount %d, keptValue %d, liveState %s\n", FILE__,
            __LINE__, reg->number.getId(), reg->refCount, reg->keptValue,
            (reg->liveState == registerSlot::live)
                ? "live"
                : ((reg->liveState == registerSlot::spilled) ? "spilled" : "dead"));

        if(reg->refCount > 0 ||                      // Currently active
           reg->keptValue ||                         // Has a kept value
           (reg->liveState == registerSlot::live)) { // needs to be saved pre-call
          regalloc_printf("%s[%d]: \tsaving reg\n", FILE__, __LINE__);
          pair<unsigned, unsigned> regToSave;
          regToSave.first = reg->number;

          regToSave.second = reg->refCount;
          // We can have both a keptValue and a refCount - so I invert
          // the refCount if there's a keptValue
          if(reg->keptValue) {
            regToSave.second *= -1;
          }

          savedRegsToRestore.push_back(regToSave);

          // The register is live; save it.
          AMD64::emitPushReg64(reg->encoding(), gen);
          // And now that it's saved, nuke it
          reg->refCount = 0;
          reg->keptValue = false;
        } else {
          // mapping from Register to MachRegister, then to index in liveness bitArray
          if(regsClobberedByCall.test(live.getIndex(regToMachReg64.equal_range(r).first->second))) {
            gen.markRegDefined(r);
          }
        }
      }
    }

    // Make sure we'll be adding exactly enough to the stack to maintain
    // alignment required by the AMD64 ABI.
    //
    // We must make sure this matches the number of push operations
    // in the operands.size() loop below.
    int stack_operands = operands.size() - AMD64_ARG_REGS;
    if(stack_operands < 0) {
      stack_operands = 0;
    }

    int alignment = (savedRegsToRestore.size() + stack_operands) * 8;
    if(alignment % AMD64_STACK_ALIGNMENT) {
      alignment = AMD64_STACK_ALIGNMENT - (alignment % AMD64_STACK_ALIGNMENT);
    }

    if(alignment) {
      emitLEA(REGNUM_RSP, Null_Register, 0, -alignment, REGNUM_RSP, gen);
      gen.rs()->incStack(alignment);
    }

    // generate code for arguments
    // Now, it would be _really_ nice to emit into
    // the correct register so we don't need to move it.
    // So try and allocate the correct one.
    // We should be able to - we saved them all up above.
    int frame_size = 0;
    for(int u = operands.size() - 1; u >= 0; u--) {
      Address unused = ADDR_NULL;
      Register reg = Null_Register;
      if(u >= (int)AMD64_ARG_REGS) {
        if(!operands[u]->generateCode_phase2(gen, noCost, unused, reg)) {
          assert(0);
        }
        assert(reg != Null_Register);
        AMD64::emitPushReg64(reg, gen);
        gen.rs()->freeRegister(reg);
        frame_size++;
      } else {
        if(gen.rs()->allocateSpecificRegister(gen, (unsigned)amd64_arg_regs[u], true)) {
          reg = amd64_arg_regs[u];
        } else {
          cerr << "Error: tried to allocate register " << amd64_arg_regs[u] << " and failed!"
               << endl;
          assert(0);
        }
        gen.markRegDefined(reg);
        if(!operands[u]->generateCode_phase2(gen, noCost, unused, reg)) {
          assert(0);
        }
        if(reg != amd64_arg_regs[u]) {
          // Code generator said "we've already got this one in a different
          // register, so just reuse it"
          AMD64::emitMovRegToReg64(amd64_arg_regs[u], reg, true, gen);
        }
      }
    }

    // RAX = number of FP regs used by varargs on AMD64 (also specified as caller-saved).
    // Clobber it to 0.
    AMD64::emitMovImmToReg64(REGNUM_RAX, 0, true, gen);
    gen.markRegDefined(REGNUM_RAX);

    emitCallInstruction(gen, callee, Null_Register);

    // Now clear whichever registers were "allocated" for a return value
    // Don't do that for stack-pushed operands; they've already been freed.
    for(unsigned i = 0; i < operands.size(); i++) {
      if(i == AMD64_ARG_REGS) {
        break;
      }

      gen.rs()->freeRegister(amd64_arg_regs[i]);
    }
    if(frame_size) {
      emitAdjustStackPointer(frame_size, gen);
      // AMD64::emitOpRegImm64(0x81, EXTENDED_0x81_ADD, REGNUM_RSP, frame_size * 8, gen); // add
      // esp, frame_size
    }

    if(alignment) {
      // Skip past the stack alignment.
      emitLEA(REGNUM_RSP, Null_Register, 0, alignment, REGNUM_RSP, gen);
      gen.rs()->incStack(-alignment);
    }

    if(!inInstrumentation) {
      return Null_Register;
    }

    // We now have a bit of an ordering problem.
    // The RS thinks all registers are free; this is not the case
    // We've saved the incoming registers, and it's likely that
    // the return value is co-occupying one.
    // We need to restore the registers, but _first_ we need to
    // restore the RS state and allocate a keeper register.
    // Now restore any registers live over the call

    for(int i = savedRegsToRestore.size() - 1; i >= 0; i--) {
      registerSlot *reg = (*gen.rs())[savedRegsToRestore[i].first];

      if(savedRegsToRestore[i].second < 1) {
        reg->refCount = -1 * (savedRegsToRestore[i].second);
        reg->keptValue = true;
      } else {
        reg->refCount = savedRegsToRestore[i].second;
      }
    }

    // allocate a (virtual) register to store the return value
    // We do this now because the state is correct again in the RS.

    Register ret = gen.rs()->allocateRegister(gen, noCost);
    gen.markRegDefined(ret);
    AMD64::emitMovRegToReg64(ret, REGNUM_EAX, true, gen);

    // Now restore any registers live over the call
    for(int i = savedRegsToRestore.size() - 1; i >= 0; i--) {
      registerSlot *reg = (*gen.rs())[savedRegsToRestore[i].first];

      AMD64::emitPopReg64(reg->encoding(), gen);
    }

    return ret;
  }

  // FIXME: comment here on the stack layout
  void EmitterAMD64::emitGetRetVal(Register dest, bool addr_of, codeGen &gen) {
    if(!addr_of) {
      emitLoadOrigRegister(REGNUM_RAX, dest, gen);
      gen.markRegDefined(dest);
      return;
    }

    // RAX isn't defined here.  See comment in EmitterIA32::emitGetRetVal
    gen.markRegDefined(REGNUM_RAX);
    stackItemLocation loc = getHeightOf(stackItem::framebase, gen);
    registerSlot *rax = (*gen.rs())[REGNUM_RAX];
    assert(rax);
    loc.offset += (rax->saveOffset * 8);
    emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
  }

  void EmitterAMD64::emitGetRetAddr(Register dest, codeGen &gen) {
    stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
    emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
  }

  void EmitterAMD64::emitGetParam(Register dest, Register param_num, instPoint::Type pt_type,
                                  opCode op, bool addr_of, codeGen &gen) {
    if(!addr_of && param_num < 6) {
      emitLoadOrigRegister(amd64_arg_regs[param_num], dest, gen);
      gen.markRegDefined(dest);
      return;
    } else if(addr_of && param_num < 6) {
      Register reg = amd64_arg_regs[param_num];
      gen.markRegDefined(reg);
      stackItemLocation loc = getHeightOf(stackItem::framebase, gen);
      registerSlot *regSlot = (*gen.rs())[reg];
      assert(regSlot);
      loc.offset += (regSlot->saveOffset * 8);
      emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
      return;
    }
    assert(param_num >= 6);
    stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
    if(!gen.bt() || gen.bt()->alignedStack) {
      // Load the original %rsp value into dest
      AMD64::emitMovRMToReg64(dest, loc.reg.reg(), loc.offset, 8, gen);
      loc.reg = RealRegister(dest);
      loc.offset = 0;
    }

    switch(op) {
      case getParamOp:
        if(pt_type == instPoint::FuncEntry) {
          // Return value before any parameters
          loc.offset += 8;
        }
        break;
      case getParamAtCallOp:
        break;
      case getParamAtEntryOp:
        loc.offset += 8;
        break;
      default:
        assert(0);
        break;
    }

    loc.offset += (param_num - 6) * 8;
    if(!addr_of) {
      AMD64::emitMovRMToReg64(dest, loc.reg.reg(), loc.offset, 8, gen);
    } else {
      emitLEA(loc.reg.reg(), Null_Register, 0, loc.offset, dest, gen);
    }
  }

  void EmitterAMD64::emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift,
                                codeGen &gen) {
    // Support for using ESP that has been moved is unimplemented.

    assert(stackShift == 0);
    Register use_a = Null_Register;
    Register use_b = Null_Register;

    bool havera = ra > -1, haverb = rb > -1;

    // if ra is specified, move its inst-point value into our
    // destination register
    gen.markRegDefined(dest);
    if(havera) {
      if(ra == mRIP) {
        // special case: rip-relative data addressing
        // the correct address has been stuffed in imm
        AMD64::emitMovImmToReg64(dest, imm, true, gen);
        return;
      }
      if(gen.inInstrumentation()) {
        use_a = dest;
        emitLoadOrigRegister(ra, dest, gen);
      } else {
        use_a = ra;
      }
    }

    // if rb is specified, move its inst-point value into RAX
    if(haverb) {
      if(gen.inInstrumentation()) {
        use_b = gen.rs()->getScratchRegister(gen);
        gen.markRegDefined(use_b);
        emitLoadOrigRegister(rb, use_b, gen);
      } else {
        use_b = rb;
      }
    }
    // emitLEA will not handle the [disp32] case properly, so
    // we special case that
    if(!havera && !haverb) {
      AMD64::emitMovImmToReg64(dest, imm, false, gen);
    } else {
      emitLEA(use_a, use_b, sc, (int)imm, dest, gen);
    }
  }

  void EmitterAMD64::emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen) {
    // count is at most 1 register or constant or hack (aka pseudoregister)
    assert((ra == -1) && ((rb == -1) || ((imm == 0) && (rb == 1 /*REGNUM_ECX */ ||
                                                        rb >= Dyninst::DyninstAPI::IA32_EMULATE))));

    gen.markRegDefined(dest);
    if(rb >= Dyninst::DyninstAPI::IA32_EMULATE) {
      // need to emulate repeated SCAS or CMPS to figure out byte count

      // TODO: firewall code to ensure that direction is up

      bool neg = false;
      unsigned char opcode_small, opcode_large;
      bool restore_rax = false;
      bool restore_rsi = false;

      bool rax_wasUsed = false;
      bool rsi_wasUsed = false;
      bool rdi_wasUsed = false;
      bool rcx_wasUsed = false;

      switch(rb) {
        case Dyninst::DyninstAPI::IA32_NESCAS:
          neg = true;
          DYNINST_FALLTHROUGH;
        case Dyninst::DyninstAPI::IA32_ESCAS:
          opcode_small = 0xAE;
          opcode_large = 0xAF;
          restore_rax = true;
          break;
        case Dyninst::DyninstAPI::IA32_NECMPS:
          neg = true;
          DYNINST_FALLTHROUGH;
        case Dyninst::DyninstAPI::IA32_ECMPS:
          opcode_small = 0xA6;
          opcode_large = 0xA7;
          restore_rsi = true;
          break;
        default:
          assert(!"Wrong emulation!");
      }

      // restore flags (needed for direction flag)
      gen.codeEmitter()->emitRestoreFlagsFromStackSlot(gen);

      // restore needed registers to values at the inst point
      // (push current values on the stack in case they're in use)
      if(restore_rax) {
        // We often use RAX as a destination register - in this case,
        // it's allocated but by us. And we really don't want to save
        // it and then restore...
        if(!gen.rs()->isFreeRegister(REGNUM_RAX) && (dest != REGNUM_RAX)) {
          rax_wasUsed = true;
          AMD64::emitPushReg64(REGNUM_RAX, gen);
        }
        emitLoadOrigRegister(REGNUM_RAX, REGNUM_RAX, gen);
      }
      if(restore_rsi) {
        if(!gen.rs()->isFreeRegister(REGNUM_RSI) && (dest != REGNUM_RSI)) {
          rsi_wasUsed = true;
          AMD64::emitPushReg64(REGNUM_RSI, gen);
        }
        emitLoadOrigRegister(REGNUM_RSI, REGNUM_RSI, gen);
      }
      if(!gen.rs()->isFreeRegister(REGNUM_RDI) && (dest != REGNUM_RDI)) {
        rdi_wasUsed = true;
        AMD64::emitPushReg64(REGNUM_RDI, gen);
      }
      emitLoadOrigRegister(REGNUM_RDI, REGNUM_RDI, gen);
      if(!gen.rs()->isFreeRegister(REGNUM_RCX) && (dest != REGNUM_RCX)) {
        rcx_wasUsed = true;
        AMD64::emitPushReg64(REGNUM_RCX, gen);
      }
      emitLoadOrigRegister(REGNUM_RCX, REGNUM_RCX, gen);

      // emulate the string instruction
      emitSimpleInsn(neg ? 0xF2 : 0xF3, gen); // rep(n)e
      if(sc == 0) {
        emitSimpleInsn(opcode_small, gen);
      } else {
        if(sc == 1) {
          emitSimpleInsn(0x66, gen); // operand size prefix
        } else if(sc == 3) {
          emitSimpleInsn(0x48, gen); // REX.W
        }
        emitSimpleInsn(opcode_large, gen);
      }

      // RCX has now been decremented by the number of repititions
      // load old RCX into RAX and compute difference
      emitLoadOrigRegister(REGNUM_RCX, dest, gen);
      emitOp(0x2B, dest, dest, REGNUM_RCX, gen);

      // restore registers we stomped on
      if(rcx_wasUsed) {
        AMD64::emitPopReg64(REGNUM_RCX, gen);
      }
      if(rdi_wasUsed) {
        AMD64::emitPopReg64(REGNUM_RDI, gen);
      }
      if(rsi_wasUsed) {
        AMD64::emitPopReg64(REGNUM_RSI, gen);
      }
      if(rax_wasUsed) {
        AMD64::emitPopReg64(REGNUM_RAX, gen);
      }
    } else if(rb > -1) {

      // count spec is simple register with scale
      // TODO: 16-bit pseudoregisters
      assert(rb < 16);

      // store the register into RAX
      Register scratch = gen.rs()->getScratchRegister(gen);
      gen.markRegDefined(scratch);
      emitLoadOrigRegister(rb, scratch, gen);

      // shift left by the given scale
      // emitTimesImm will do the right thing
      if(sc > 0) {
        emitTimesImm(dest, scratch, 1 << sc, gen);
      }
    } else {
      AMD64::emitMovImmToReg64(dest, (int)imm, true, gen);
    }
  }

  void EmitterAMD64::emitRestoreFlags(codeGen &gen, unsigned offset) {
    if(offset) {
      emitOpRMReg(PUSH_RM_OPC1, RealRegister(REGNUM_EBP), offset * 8, RealRegister(PUSH_RM_OPC2),
                  gen);
    }
    emitSimpleInsn(0x9D, gen);
  }

  void EmitterAMD64::emitPushFlags(codeGen &gen) {
    // save flags (PUSHFQ)
    emitSimpleInsn(0x9C, gen);
  }

  void EmitterAMD64::emitRestoreFlagsFromStackSlot(codeGen &gen) {
    stackItemLocation loc = getHeightOf(stackItem(RealRegister(REGNUM_OF)), gen);
    emitOpRMReg(PUSH_RM_OPC1, RealRegister(loc.reg.reg()), loc.offset, RealRegister(PUSH_RM_OPC2),
                gen);
    emitSimpleInsn(0x9D, gen);
  }

  bool shouldSaveReg(registerSlot *reg, baseTramp *inst, bool saveFlags) {
    if(reg->encoding() == REGNUM_RSP) {
      return false;
    }

    if(inst->point()) {
      regalloc_printf("\t shouldSaveReg for BT %p, from 0x%lx\n", (void *)inst,
                      inst->point()->insnAddr());
    } else {
      regalloc_printf("\t shouldSaveReg for iRPC\n");
    }
    if(reg->liveState != registerSlot::live) {
      regalloc_printf("\t Reg %u not live, concluding don't save\n", reg->number.getId());
      return false;
    }
    if(saveFlags) {
      // Saving flags takes up EAX/RAX, and so if they're live they must
      // be saved even if we don't explicitly use them
      DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_LOGICAL_OP
      if(reg->number == REGNUM_EAX || reg->number == REGNUM_RAX) {
        return true;
      }
      DYNINST_DIAGNOSTIC_END_SUPPRESS_LOGICAL_OP
    }
    if(inst && inst->validOptimizationInfo() && !inst->definedRegs[reg->encoding()]) {
      regalloc_printf(
          "\t Base tramp instance doesn't have reg %u (num %u) defined; concluding don't save\n",
          reg->encoding(), reg->number.getId());
      return false;
    }
    return true;
  }

  // Moves stack pointer by offset and aligns it to AMD64_STACK_ALIGNMENT
  // with the following sequence:
  //
  //     lea    -off(%rsp) => %rsp           # move %rsp down
  //     mov    %rax => saveSlot1(%rsp)      # save %rax onto stack
  //     lahf                                # save %rflags byte into %ah
  //     seto   %al                          # save overflow flag into %al
  //     mov    %rax => saveSlot2(%rsp)      # save flags %rax onto stack
  //     lea    off(%rsp) => %rax            # store original %rsp in %rax
  //     and    -$AMD64_STACK_ALIGNMENT,%rsp # align %rsp
  //     mov    %rax => (%rsp)               # store original %rsp on stack
  //     mov    -off+saveSlot2(%rax) => %rax # restore flags %rax from stack
  //     add    $0x7f,%al                    # restore overflow flag from %al
  //     sahf                                # restore %rflags byte from %ah
  //     mov    (%rsp) => %rax               # re-load old %rsp into %rax to ...
  //     mov    -off+saveSlot1(%rax) => %rax # ... restore %rax from stack
  //
  // This sequence has four important properties:
  //     1) It never writes to memory within offset bytes below the original
  //        %rsp.  This is to make it compatible with red zone skips.
  //     2) It never *directly* writes to memory below %rsp.  It always begins
  //        by moving %rsp down, then writing to locations above it.  This way,
  //        if the kernel decides to interrupt, it won't stomp all over our
  //        values before we get a chance to use them.
  //     3) It is designed to support easy de-allocation of this space by
  //        ending with %rsp pointing to where we stored the original %rsp.
  //     4) Care has been taken to properly restore both %eax and %eflags
  //        by using "lea" instead of "add" or "sub," and saving the necessary
  //        flags around the "and" instruction.
  //
  // Saving of the flags register can be skipped if the register is not live.

  void EmitterAMD64::emitStackAlign(int offset, codeGen &gen) {
    int off = offset + 8 + AMD64_STACK_ALIGNMENT;
    int saveSlot1 = 0 + AMD64_STACK_ALIGNMENT;
    int saveSlot2 = 8 + AMD64_STACK_ALIGNMENT;

    bool saveFlags = false;
    Register scratch = REGNUM_RAX;

    if(gen.rs()->checkVolatileRegisters(gen, registerSlot::live)) {
      saveFlags = true; // We need to save the flags register
      off += 8;         // Allocate stack space to store the flags
    }

    emitLEA(REGNUM_RSP, Null_Register, 0, -off, REGNUM_RSP, gen);
    emitStoreRelative(scratch, saveSlot1, REGNUM_RSP, 8, gen);
    if(saveFlags) {
      emitSimpleInsn(0x9f, gen);
      emitSaveO(gen);
      emitStoreRelative(scratch, saveSlot2, REGNUM_RSP, 8, gen);
    }
    emitLEA(REGNUM_RSP, Null_Register, 0, off, scratch, gen);

    AMD64::emitOpRegImm8_64(0x83, EXTENDED_0x83_AND, REGNUM_RSP, -AMD64_STACK_ALIGNMENT, true, gen);
    emitStoreRelative(scratch, 0, REGNUM_RSP, 8, gen);
    if(saveFlags) {
      emitLoadRelative(scratch, -off + saveSlot2, scratch, 8, gen);
      emitRestoreO(gen);
      emitSimpleInsn(0x9e, gen);
      emitLoadRelative(scratch, 0, REGNUM_RSP, 8, gen);
    }
    emitLoadRelative(scratch, -off + saveSlot1, scratch, 8, gen);
  }

  bool EmitterAMD64::emitBTSaves(baseTramp *bt, codeGen &gen) {
    gen.setInInstrumentation(true);

    int instFrameSize = 0; // Tracks how much we are moving %rsp

    // Align the stack now to avoid having a padding hole in the middle of
    // our instrumentation stack.  Referring to anything on the stack above
    // this point will require an indirect reference.
    //
    // There are four cases that require a AMD64_STACK_ALIGNMENT aligned
    // stack pointer:
    //
    //    - Any time we need to save the FP registers
    //    - Any time we call a function (Required by the AMD64 ABI)
    //    - Any time we may execute SSE/SSE2 instructions
    //
    // The third case is equivalent to the second case, so search the
    // ASTs for function call generation.
    //
    bool useFPRs = BPatch::bpatch->isForceSaveFPROn() ||
                   (BPatch::bpatch->isSaveFPROn() && gen.rs()->anyLiveFPRsAtEntry() &&
                    // bt->saveFPRs()               &&
                    bt->makesCall());
    bool alignStack = useFPRs || !bt || bt->checkForFuncCalls();
    bool saveFlags = gen.rs()->checkVolatileRegisters(gen, registerSlot::live);
    bool createFrame = !bt || bt->needsFrame() || useFPRs;
    bool saveOrigAddr = createFrame && bt->instP();
    // Stores the offset to the location of the previous SP stored
    // in the stack when a frame is created.
    uint64_t sp_offset = 0;
    int num_saved = 0;
    int num_to_save = 0;
    // Calculate the number of registers we'll save
    for(int i = 0; i < gen.rs()->numGPRs(); i++) {
      registerSlot *reg = gen.rs()->GPRs()[i];
      if(!shouldSaveReg(reg, bt, saveFlags)) {
        continue;
      }
      if(createFrame && reg->encoding() == REGNUM_RBP) {
        continue;
      }
      num_to_save++;
    }
    if(createFrame) {
      num_to_save++; // will save rbp
      num_to_save++; // Saving SP
      num_to_save++; // Saving Flag Variable
    }
    if(saveOrigAddr) {
      num_to_save++; // Stack slot for return value, no actual save though
    }
    if(saveFlags) {
      num_to_save++;
    }

    bool skipRedZone = (num_to_save > 0) || alignStack || saveOrigAddr || createFrame;

    if(alignStack) {
      emitStackAlign(AMD64_RED_ZONE, gen);
    } else if(skipRedZone) {
      // Just move %rsp past the red zone
      // Use LEA to avoid flag modification.
      emitLEA(REGNUM_RSP, Null_Register, 0, -AMD64_RED_ZONE, REGNUM_RSP, gen);
      instFrameSize += AMD64_RED_ZONE;
      // In cases where redzone offset is skipped without alignment
      // the previous frame's SP is just the redzone skip + REG_COUNT * 8
      sp_offset += AMD64_RED_ZONE;
    }

    // Save the live ones
    for(int i = 0; i < gen.rs()->numGPRs(); i++) {
      registerSlot *reg = gen.rs()->GPRs()[i];

      if(!shouldSaveReg(reg, bt, saveFlags)) {
        continue;
      }
      if(createFrame && reg->encoding() == REGNUM_RBP) {
        continue;
      }
      AMD64::emitPushReg64(reg->encoding(), gen);
      // We move the FP down to just under here, so we're actually
      // measuring _up_ from the FP.
      assert((18 - num_saved) > 0);
      num_saved++;
      gen.rs()->markSavedRegister(reg->encoding(), num_to_save - num_saved);
    }

    // Save flags if we need to
    if(saveFlags) {
      gen.rs()->saveVolatileRegisters(gen);
      AMD64::emitPushReg64(REGNUM_RAX, gen);

      num_saved++;
      gen.rs()->markSavedRegister(REGNUM_EFLAGS, num_to_save - num_saved);
      // Need a "defined, but not by us silly"
      gen.markRegDefined(REGNUM_RAX);
    }

    if(createFrame) {
      Register itchy = gen.rs()->getScratchRegister(gen);

      // add an offset each register saved so far.
      sp_offset += (8 * (num_saved));
      // If stack alignment is used, pull the original SP from the stack
      // this location is sp_offset.
      if(alignStack) {
        emitLoadRelative(itchy, sp_offset, REGNUM_RSP, 8, gen);
        AMD64::emitPushReg64(itchy, gen);
      } else {
        // Otherwise, the previous SP is exactly SP+sp_offset away
        emitLEA(REGNUM_RSP, Null_Register, 0, sp_offset, itchy, gen);
        AMD64::emitPushReg64(itchy, gen);
      }
      // Special Word to help stackwalker know in First Party mode that
      // it is attempting to walk out of an inst frame.
      AMD64::emitMovImmToReg64(itchy, 0xBEEFDEAD, true, gen);
      AMD64::emitPushReg64(itchy, gen);
      gen.rs()->freeRegister(itchy);
    }

    // push a return address for stack walking
    if(saveOrigAddr) {
      Register origTmp = gen.rs()->getScratchRegister(gen);
      AMD64::emitMovImmToReg64(origTmp, bt->instP()->addr_compat(), true, gen);
      AMD64::emitPushReg64(origTmp, gen);
      gen.markRegDefined(origTmp);
      num_saved++;
    }

    // Push RBP...
    if(createFrame) {

      // set up a fresh stack frame
      // pushl %rbp        (0x55)
      // movl  %rsp, %rbp  (0x48 0x89 0xe5)
      emitSimpleInsn(0x55, gen);
      gen.rs()->markSavedRegister(REGNUM_RBP, 0);
      num_saved++;
      num_saved++;
      num_saved++;
      // And track where it went
      (*gen.rs())[REGNUM_RBP]->liveState = registerSlot::spilled;
      (*gen.rs())[REGNUM_RBP]->spilledState = registerSlot::framePointer;
      (*gen.rs())[REGNUM_RBP]->saveOffset = 0;

      AMD64::emitMovRegToReg64(REGNUM_RBP, REGNUM_RSP, true, gen);
    }

    assert(num_saved == num_to_save);

    // Prepare our stack bookkeeping data structures.
    instFrameSize += num_saved * 8;
    if(bt) {
      bt->stackHeight = instFrameSize;
    }
    gen.rs()->setInstFrameSize(instFrameSize);
    gen.rs()->setStackHeight(0);

    // Pre-calculate space for re-alignment and floating-point state.
    int extra_space = 0;
    if(useFPRs) {
      extra_space += 512;
    }

    // Make sure that we're still 32-byte aligned when we add extra_space
    // to the stack.
    if(alignStack) {
      if((instFrameSize + extra_space) % 32) {
        extra_space += 32 - ((instFrameSize + extra_space) % 32);
      }
    }

    if(extra_space) {
      emitLEA(REGNUM_RSP, Null_Register, 0, -extra_space, REGNUM_RSP, gen);
      gen.rs()->incStack(extra_space);
    }
    extra_space_check = extra_space;

    bool needFXsave = false;
    if(useFPRs) {
      // need to save the floating point state (x87, MMX, SSE)
      // Since we're guarenteed to be at least 16-byte aligned
      // now, the following sequence does the job:
      //
      //   fxsave (%rsp)           ; 0x0f 0xae 0x04 0x24

      // Change to REGET if we go back to magic LEA emission

      for(auto curReg = gen.rs()->FPRs().begin(); curReg != gen.rs()->FPRs().end(); ++curReg) {
        if((*curReg)->liveState != registerSlot::dead) {
          switch((*curReg)->number) {
            case REGNUM_XMM0:
            case REGNUM_XMM1:
            case REGNUM_XMM2:
            case REGNUM_XMM3:
            case REGNUM_XMM4:
            case REGNUM_XMM5:
            case REGNUM_XMM6:
            case REGNUM_XMM7:
              continue;
            default:
              needFXsave = true;
              break;
          }
        }
      }

      if(needFXsave) {
        GET_PTR(buffer, gen);
        *buffer++ = 0x0f;
        *buffer++ = 0xae;
        *buffer++ = 0x04;
        *buffer++ = 0x24;
        SET_PTR(buffer, gen);
      } else {
        AMD64::emitMovRegToReg64(REGNUM_RAX, REGNUM_RSP, true, gen);
        AMD64::emitXMMRegsSaveRestore(gen, false);
      }
    }

    if(bt) {
      bt->savedFPRs = useFPRs;
      bt->wasFullFPRSave = needFXsave;

      bt->createdFrame = createFrame;
      bt->savedOrigAddr = saveOrigAddr;
      bt->createdLocalSpace = false;
      bt->alignedStack = alignStack;
      bt->savedFlags = saveFlags;
      bt->skippedRedZone = skipRedZone;
    }

    return true;
  }

  bool EmitterAMD64::emitBTRestores(baseTramp *bt, codeGen &gen) {
    bool useFPRs = false;
    bool createFrame = false;
    bool saveOrigAddr = false;
    bool alignStack = false;
    bool skippedRedZone = false;
    bool restoreFlags = false;

    if(bt) {
      useFPRs = bt->savedFPRs;
      createFrame = bt->createdFrame;
      saveOrigAddr = bt->savedOrigAddr;
      alignStack = bt->alignedStack;
      skippedRedZone = bt->skippedRedZone;
      restoreFlags = bt->savedFlags;
    } else {
      useFPRs = BPatch::bpatch->isForceSaveFPROn() ||
                (BPatch::bpatch->isSaveFPROn() && gen.rs()->anyLiveFPRsAtEntry());
      createFrame = true;
      saveOrigAddr = false;
      alignStack = true;
      skippedRedZone = true; // Obviated by alignStack, but hey
      restoreFlags = true;
    }

    if(useFPRs) {
      // restore saved FP state
      // fxrstor (%rsp) ; 0x0f 0xae 0x04 0x24
      if(bt && bt->wasFullFPRSave) {
        GET_PTR(buffer, gen);
        *buffer++ = 0x0f;
        *buffer++ = 0xae;
        *buffer++ = 0x0c;
        *buffer++ = 0x24;
        SET_PTR(buffer, gen);
      } else {
        AMD64::emitMovRegToReg64(REGNUM_RAX, REGNUM_RSP, true, gen);
        AMD64::emitXMMRegsSaveRestore(gen, true);
      }
    }

    int extra_space = gen.rs()->getStackHeight();
    assert(extra_space == extra_space_check);
    if(!createFrame && extra_space) {
      emitLEA(REGNUM_RSP, Null_Register, 0, extra_space, REGNUM_RSP, gen);
    }

    if(createFrame) {
      // tear down the stack frame (LEAVE)
      emitSimpleInsn(0xC9, gen);
      // Pop the Previous SP and Special Word off of the stack, discard them
      Register itchy = gen.rs()->getScratchRegister(gen);
      AMD64::emitPopReg64(itchy, gen);
      AMD64::emitPopReg64(itchy, gen);
      gen.rs()->freeRegister(itchy);
    }

    // pop "fake" return address
    if(saveOrigAddr) {
      AMD64::emitPopReg64(REGNUM_RAX, gen);
    }

    // Restore flags
    if(restoreFlags) {
      AMD64::emitPopReg64(REGNUM_RAX, gen);
      gen.rs()->restoreVolatileRegisters(gen);
    }

    // restore saved registers
    for(int i = gen.rs()->numGPRs() - 1; i >= 0; i--) {
      registerSlot *reg = gen.rs()->GPRs()[i];
      if(reg->encoding() == REGNUM_RBP && createFrame) {
        // Although we marked it saved, we already restored it
        // above.
        continue;
      }

      if(reg->liveState == registerSlot::spilled) {
        AMD64::emitPopReg64(reg->encoding(), gen);
      }
    }

    // Restore the (possibly unaligned) stack pointer.
    if(alignStack) {
      emitLoadRelative(REGNUM_RSP, 0, REGNUM_RSP, 0, gen);
    } else if(skippedRedZone) {
      emitLEA(REGNUM_ESP, Null_Register, 0, AMD64_RED_ZONE, REGNUM_ESP, gen);
    }

    gen.setInInstrumentation(false);
    return true;
  }

  void EmitterAMD64::emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost) {
    if(!AMD64::isImm64bit(addr) && !AMD64::isImm64bit(imm)) {
      emitMovImmToMem(addr, imm, gen);
    } else {
      Register r = gen.rs()->allocateRegister(gen, noCost);
      gen.markRegDefined(r);
      AMD64::emitMovImmToReg64(r, addr, true, gen);
      AMD64::emitMovImmToRM64(r, 0, imm, true, gen);
      gen.rs()->freeRegister(r);
    }
  }

  void EmitterAMD64::emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost) {
    if(!AMD64::isImm64bit(addr) && !AMD64::isImm64bit(imm)) {
      x86::emitAddMem(addr, imm, gen);
    } else {
      Register r = gen.rs()->allocateRegister(gen, noCost);
      gen.markRegDefined(r);
      AMD64::emitMovImmToReg64(r, addr, true, gen);
      AMD64::emitAddRM64(r, imm, true, gen);
      gen.rs()->freeRegister(r);
    }
  }

  bool EmitterAMD64::emitPush(codeGen &gen, Register reg) {
    AMD64::emitPushReg64(reg, gen);
    return true;
  }

  bool EmitterAMD64::emitPop(codeGen &gen, Register reg) {
    AMD64::emitPopReg64(reg, gen);
    return true;
  }

  bool EmitterAMD64::emitAdjustStackPointer(int index, codeGen &gen) {
    // The index will be positive for "needs popped" and negative
    // for "needs pushed". However, positive + SP works, so don't
    // invert.
    int popVal = index * gen.addrSpace()->getAddressWidth();
    AMD64::emitOpRegImm64(0x81, EXTENDED_0x81_ADD, REGNUM_ESP, popVal, true, gen);
    gen.rs()->incStack(-1 * popVal);
    return true;
  }

  void EmitterAMD64::emitLoadShared(opCode op, Register dest, const image_variable *var,
                                    bool is_local, int size, codeGen &gen, Address offset) {
    Address addr;
    gen.markRegDefined(dest);
    if(!var) {
      addr = offset;
    } else if(is_local) {
      addr = (Address)var->getOffset();
    } else {
      // create or retrieve jump slot
      addr = getInterModuleVarAddr(var, gen);
    }

    if(op == loadConstOp) {
      int addr_offset = addr - gen.currAddr();
      // Brutal hack for IP-relative: displacement operand on 32-bit = IP-relative on 64-bit.
      if(is_local || !var) {
        emitLEA(Null_Register, Null_Register, 0, addr_offset - 7, dest, gen);
      } else {
        AMD64::emitMovPCRMToReg64(dest, addr - gen.currAddr(), 8, gen, true);
      }

      return;
    }

    // load register with address from jump slot
    if(!is_local) {
      AMD64::emitMovPCRMToReg64(dest, addr - gen.currAddr(), 8, gen, true);
      emitLoadIndir(dest, dest, size, gen);
    } else {
      AMD64::emitMovPCRMToReg64(dest, addr - gen.currAddr(), size, gen, true);
    }
  }

  void EmitterAMD64::emitStoreShared(Register source, const image_variable *var, bool is_local,
                                     int size, codeGen &gen) {
    Address addr;

    if(is_local) {
      addr = (Address)var->getOffset();
    } else {
      addr = getInterModuleVarAddr(var, gen);
    }

    // temporary virtual register for storing destination address
    Register dest = gen.rs()->allocateRegister(gen, false);
    gen.markRegDefined(dest);

    // load register with address from jump slot
    emitLEA(Null_Register, Null_Register, 0, addr - gen.currAddr() - 7, dest, gen);
    // emitMovPCRMToReg64(dest, addr-gen.currAddr(), gen, true);
    if(!is_local) {
      emitLoadIndir(dest, dest, 8, gen);
    }

    // get the variable with an indirect load
    emitStoreIndir(dest, source, size, gen);

    gen.rs()->freeRegister(dest);
  }

  bool EmitterAMD64::emitXorRegRM(Register dest, Register base, int disp, codeGen &gen) {
    AMD64::emitOpRegRM64(XOR_R32_RM32, dest, base, disp, true, gen);
    gen.markRegDefined(dest);
    return true;
  }

  bool EmitterAMD64::emitXorRegReg(Register dest, Register base, codeGen &gen) {
    AMD64::emitOpRegReg64(XOR_R32_RM32, dest, base, true, gen);
    gen.markRegDefined(dest);
    return true;
  }

  bool EmitterAMD64::emitXorRegImm(Register dest, int imm, codeGen &gen) {
    AMD64::emitOpRegImm64(0x81, 6, dest, imm, false, gen);
    gen.markRegDefined(dest);
    return true;
  }

  bool EmitterAMD64::emitXorRegSegReg(Register dest, Register base, int disp, codeGen &gen) {
    Register tmp_dest = dest;
    Register tmp_base = base;

    x86::emitSegPrefix(base, gen);
    AMD64::emitRex(true, &tmp_dest, NULL, &tmp_base, gen);
    emitOpSegRMReg(XOR_R32_RM32, RealRegister(tmp_dest), RealRegister(tmp_base), disp, gen);
    gen.markRegDefined(dest);
    return true;
  }

}}
