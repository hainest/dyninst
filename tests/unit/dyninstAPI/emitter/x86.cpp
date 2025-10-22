#include "Architecture.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/registerSpace.h"
#include "emitter_test.h"

int main() {
  using Dyninst::verify_emitter;
  using Dyninst::emitter_buffer_t;

  constexpr auto arch = Dyninst::Arch_x86;
  auto const size = Dyninst::getArchAddressWidth(arch);
  auto *rs = registerSpace::getRegisterSpace(size);

  BinaryEdit bin_edit{};

  constexpr auto buffer_size = 128;
  codeGen gen(buffer_size);
  gen.setAddrSpace(&bin_edit);
  gen.setRegisterSpace(rs);
  
  auto *emitter = static_cast<EmitterIA32Stat*>(gen.emitter());

  bool failed = false;

  // Move value from ebp to eax
  emitter->emitMoveRegToReg(REGNUM_EBP, REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<2>{{
    0x8b, 0xc5  // mov eax, ebp
  }});
  
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
  
  // Load immediate into eax
  emitter->emitLoadConst(REGNUM_EAX, 0x12345678, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<5>{{
    0xb8, 0x78, 0x56, 0x34, 0x12  // mov eax, 0x12345678
  }});

  // Store an immediate value at a fixed address
  emitter->emitStoreImm(0x12345678, 0x9abcdef, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<11>{{
    // mov dword ptr [0x12345678], 0x9abcdef
    0xc7, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12, 0xef, 0xcd, 0xab, 0x09
  }});

  // Load value from fixed address into eax
  emitter->emitLoad(REGNUM_EAX, 0x12345678, 4, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<6>{{
    0x8b, 0x05, 0x78, 0x56, 0x34, 0x12    // mov eax, dword ptr [0x12345678]
  }});

  // Store at a fixed address the value contained in eax
  emitter->emitStore(0x12345678, REGNUM_EAX, 0x4, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<6>{{
    0x89, 0x05, 0x78, 0x56, 0x34, 0x12    // mov dword ptr [0x12345678], eax
  }});

  //// Load value into eax using register-indirect addressing w/o displacement
  //emitter->emitLoadIndir(REGNUM_EAX, REGNUM_ESI, 0x4, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<2>{{
    //0x8b, 0x06    // mov eax, dword ptr [esi]
  //}});

  //// Store value contained in eax using register-indirect addressing w/o displacement
  //emitter->emitStoreIndir(REGNUM_EBP, REGNUM_EAX, 0x8, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    //0x48, 0x89, 0x45, 0x0   // mov dword ptr [ebp], eax
  //}});

  //// Load value into eax using direct addressing with with segment override
  //emitter->emitLoadRelativeSegReg(REGNUM_EAX, 0x12345678, REGNUM_GS, 0x4, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<9>{{
    //0x65, 0x8b, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12   // mov eax, dword ptr gs:[0x12345678]
  //}});

  // Multiply eax by 3
  emitter->emitTimesImm(REGNUM_EAX, REGNUM_EAX, 0x03, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<6>{{
    0x69, 0xc0, 0x03, 0x0, 0x0, 0x0   // imul eax, eax, 3
  }});

/* These fail with

  dyninstAPI/src/registerSpace.C:1137:
    RealRegister registerSpace::loadVirtualForWrite(registerSlot*, codeGen&):
    Assertion `virt_r' failed.

  //  Divide edx by ecx
  emitter->emitDiv(REGNUM_EAX, REGNUM_EDX, REGNUM_ECX, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0x48, 0x8b, 0xc2,   // mov eax, edx
    0x48, 0x99,         // cqo  # sign-extend eax
    0x48, 0xf7, 0xf1    // div ecx
  }});

  // Divide esi by 0x12345678, store results in rdx
  emitter->emitDivImm(REGNUM_EDX, REGNUM_ESI, 0x12345678, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<29>{{
    0x48, 0x8b, 0xc6,                                     // mov eax, esi
    0x48, 0xba, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,   // mov rdx, 0x0
    0x68, 0x78, 0x56, 0x34, 0x12,                         // push 0x12345678
    0x48, 0xf7, 0x34, 0x24,                               // div dword ptr [esp]
    0x48, 0x8b, 0xd0,                                     // mov rdx, eax
    0x48, 0x83, 0xc4, 0x08                                // add esp, 8
    }});
*/

  // pushfq
  emitter->emitPushFlags(gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{0x9c}});

  // push eax
  emitter->emitPush(gen, REGNUM_EAX);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{0x50}});

  // pop eax
  emitter->emitPop(gen, REGNUM_EAX);
  failed |= !verify_emitter(gen, emitter_buffer_t<1>{{0x58}});

  ////  Move stack pointer by 0x20 and align it to AMD64_STACK_ALIGNMENT
  //emitter->emitStackAlign(0x20, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<48>{{
    //0x48, 0x8d, 0x64, 0x24, 0xb0,       // lea	esp, [esp - 0x50]
    //0x48, 0x89, 0x44, 0x24, 0x20,       // mov	dword ptr [esp + 0x20], eax
    //0x9f,                               // lahf
    //0x0f, 0x90, 0xc0,                   // seto	al
    //0x48, 0x89, 0x44, 0x24, 0x28,       // mov	dword ptr [esp + 0x28], eax
    //0x48, 0x8d, 0x44, 0x24, 0x50,       // lea	eax, [esp + 0x50]
    //0x48, 0x83, 0xe4, 0xe0,             // and	esp, 0xffffffffffffffe0
    //0x48, 0x89, 0x04, 0x24,             // mov	dword ptr [esp], eax
    //0x48, 0x8b, 0x40, 0xd8,             // mov	eax, dword ptr [eax - 0x28]
    //0x80, 0xc0, 0x7f,                   // add	al, 0x7f
    //0x9e,                               // sahf
    //0x48, 0x8b, 0x04, 0x24,             // mov	eax, dword ptr [esp]
    //0x48, 0x8b, 0x40, 0xd0,             // mov	eax, dword ptr [eax - 0x30]
  //}});

  //// Restore eflags from stack
  //emitter->emitRestoreFlagsFromStackSlot(gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    //0xff, 0x75, 0x0, 0x9d   // push dword ptr [ebp]
  //}});

  //  lea eax, dword ptr [esi+rbx*4+0x12345678]
  emitter->emitLEA(REGNUM_ESI, REGNUM_EBX, 0X2, 0X12345678, REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
    0x8d, 0x84, 0x9e, 0x78, 0x56, 0x34, 0x12
  }});

  // xor eax, rcx
  emitter->emitXorRegReg(REGNUM_EAX, REGNUM_ECX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<2>{{
    0x33, 0xc1
  }});

  // xor eax, dword ptr [ecx+0x12]
  emitter->emitXorRegRM(REGNUM_EAX, REGNUM_ECX, 0x12, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<3>{{
    0x33, 0x41, 0x12
  }});

  // xor eax, 0x12345678
  emitter->emitXorRegImm(REGNUM_EAX, 0x12345678, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<6>{{
    0x81, 0xf0, 0x78, 0x56, 0x34, 0x12
  }});

  //// xor eax, dword ptr gs:[0x0000000012345678]
  //emitter->emitXorRegSegReg(REGNUM_EAX, REGNUM_GS, 0x12345678, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<9>{{
    //0x65, 0x33, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12
  //}});

  // add esp, 0x91A0
  emitter->emitAdjustStackPointer(0x1234, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<6>{{
    0x81, 0xc4, 0xa0, 0x91, 0x0, 0x0
  }});

  // add dword ptr [0x12345678], 0x04
  emitter->emitAddSignedImm(0x12345678, 0x4, gen, false);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0x83, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12, 0x04
  }});

  // Load BPatch_countSpec_NP (I don't actually know what this is supposed to do)
  emitter->emitCSload(-1, -1, 4, 0x1234, REGNUM_EAX, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<5>{{
    0xb8, 0x34, 0x12, 0x0, 0x0   // mov eax, 0x1234
  }});

  //// Load BPatch_addrSpec_NP (I don't actually know what this is supposed to do)
  //emitter->emitASload(-1, -1, 4, 0x1234, REGNUM_EAX, 0x0, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<5>{{
    //0xb8, 0x34, 0x12, 0x0, 0x0   // mov eax, 0x1234
  //}});

  //// Load return value into ABI return register (eax)
  //emitter->emitGetRetVal(REGNUM_ESI, false, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<2>{{
    //0x8b, 0xf0    // mov esi, eax
  //}});

  //// Load return address from stack into esi
  //emitter->emitGetRetAddr(REGNUM_ESI, gen);
  //failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    //0x8d, 0x75, 0x00    // lea esi, dword ptr [ebp]
  //}});

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*

These require a valid baseTramp (codeGen::bt())

  void emitLoadEffectiveAddress(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen &gen);
  void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
  bool emitBTRestores(baseTramp* bt, codeGen &gen);
  bool emitBTSaves(baseTramp* bt, codeGen &gen);
  void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
  void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
  void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);
  void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
  void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);
  void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);

These require additional mocking to test.

  bool clobberAllFuncCall(registerSpace *rs,func_instance *callee);
  Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee);
  bool emitCallCleanup(codeGen &gen, func_instance *target, int frame_size, std::vector<Register> &extra_saves);
  bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret);
  int emitCallParams(codeGen &gen, const std::vector<AstNodePtr> &operands, func_instance *target, std::vector<Register> &extra_saves, bool noCost);
  void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
  bool emitPLTCall(func_instance *dest, codeGen &gen);
  bool emitPLTJump(func_instance *dest, codeGen &gen);
  void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local,int size, codeGen &gen, Address offset);
  void emitStoreShared(Register source, const image_variable *var, bool is_local,int size, codeGen &gen);

These need to expand over many 'opcode' values

  void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);
  void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm, codeGen &gen);
  void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);
  void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);

These are not implemented

  bool emitCallRelative(Register, Address, Register, codeGen &);
  bool emitMoveRegToReg(registerSlot* src, registerSlot* dest, codeGen& gen);
  bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen);
  void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
  void emitRestoreFlags(codeGen &gen, unsigned offset); Use removed by a7264f555

*/
