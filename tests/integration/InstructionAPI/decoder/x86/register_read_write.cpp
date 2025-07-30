#include "Architecture.h"
#include "decode_verify.h"
#include "InstructionDecoder.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

bool run_tests(Dyninst::Architecture, std::vector<di::rw_test_t> const &);
std::vector<di::rw_test_t> make_64only_tests();
std::vector<di::rw_test_t> make_tests(Dyninst::Architecture);

int main() {
  bool ok = run_tests(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));
  if(!run_tests(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }
  if(!run_tests(Dyninst::Arch_x86_64, make_64only_tests())) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run_tests(Dyninst::Architecture arch, std::vector<di::rw_test_t> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = (arch == Dyninst::Arch_x86) ? "x86" : "x86_64";
  for(auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.opcode.data(), t.opcode.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    auto const ok = verify_read_write_sets(insn, t.read, t.written);
    if(!ok) {
      std::cerr << sarch << " test " << test_id << " failed\n";
      failed = true;
    }
  }
  return !failed;
}

std::vector<di::rw_test_t> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto ax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;
  auto bx = is_64 ? Dyninst::x86_64::rbx : Dyninst::x86::ebx;
  auto cx = is_64 ? Dyninst::x86_64::rcx : Dyninst::x86::ecx;
  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto df = is_64 ? Dyninst::x86_64::df : Dyninst::x86::df;
  auto al = is_64 ? Dyninst::x86_64::al : Dyninst::x86::al;
  auto bp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;
  auto dl = is_64 ? Dyninst::x86_64::dl : Dyninst::x86::dl;
  auto gs = is_64 ? Dyninst::x86_64::gs : Dyninst::x86::gs;
  auto ds = is_64 ? Dyninst::x86_64::ds : Dyninst::x86::ds;
  auto si = is_64 ? Dyninst::x86_64::rsi : Dyninst::x86::esi;
  auto xmm0 = is_64 ? Dyninst::x86_64::xmm0 : Dyninst::x86::xmm0;
  auto xmm1 = is_64 ? Dyninst::x86_64::xmm1 : Dyninst::x86::xmm1;
  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;

  auto ax16 = is_64 ? Dyninst::x86_64::ax : Dyninst::x86::ax;
  auto ax32 = is_64 ? Dyninst::x86_64::eax : Dyninst::x86::eax;
  auto bp32 = is_64 ? Dyninst::x86_64::ebp : Dyninst::x86::ebp;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);
  auto ip = Dyninst::MachRegister::getPC(arch);

  using rs_t = Dyninst::register_set;

  // clang-format off
  return {
    { // add eax, 0xdeadbeef
       {0x05, 0xef, 0xbe, 0xad, 0xde},
       rs_t{ax32},
       rs_t{ax32, af, zf, of, pf, sf, cf, flags}
    },
    {// push eax
     {0x50},
     rs_t{sp, ax},
     rs_t{sp}
    },
    {// je 0x12
     {0x74, 0x10},
     rs_t{zf, sf, cf, pf, of, ip, flags},
     rs_t{ip}
    },
    {// call 0x25
     {0xe8, 0x20, 0x00, 0x00, 0x00},
     rs_t{sp, ip},
     rs_t{sp, ip}
    },
    {// clc
     {0xf8},
     rs_t{},
     rs_t{cf, flags}
    },
    {// add al, 0x30
     {0x04, 0x30},
     rs_t{al},
     rs_t{al, zf, cf, sf, of, pf, af, flags}
    },
    {// mov dword ptr [ebp - 4], 1
     {0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00},
     rs_t{bp},
     rs_t{}
    },
    {// mov byte ptr [ebp - 0x34], dl
     {0x88, 0x55, 0xcc},
     rs_t{bp, dl},
     rs_t{}
    },
    {// movddup xmm0, xmm0
     {0xf2, 0x0f, 0x12, 0xc0},
     rs_t{xmm0},
     rs_t{xmm0}
    },
    {// haddpd xmm1, xmm1
     {0x66, 0x0f, 0x7c, 0xc9},
     rs_t{xmm1},
     rs_t{xmm1}
    },
    {// lea eax, [ebx - 0xe8]
     {0x8d, 0x83, 0x18, 0xff, 0xff, 0xff},
     rs_t{bx},
     rs_t{ax32}
    },
    {// mov ax, gs
     {0x66, 0x8c, 0xe8},
     rs_t{gs},
     rs_t{ax16}
    },
    {// mov eax, ebp
     {0x89, 0xe8},
     rs_t{bp32},
     rs_t{ax32}
    },
    {// call DWORD PTR gs:[0x10]
     {0x65, 0x66, 0xff, 0x1c, 0x25, 0x10, 0x00, 0x00, 0x00},
     rs_t{gs, sp},
     rs_t{sp, ip}
    },
    {// rep lodsb al, byte ptr [esi]
     {0xf3, 0xac},
     rs_t{si, ds, flags, cx, df},
     rs_t{al, si, cx}
    },
    {// call dword ptr gs:[eax + 0x10]
     {0x65, 0xff, 0x50, 0x10},
     rs_t{ax, sp, gs, ip},
     rs_t{sp, ip}
    }
  };
  // clang-format on
}

std::vector<di::rw_test_t> make_64only_tests() {
  using rs_t = Dyninst::register_set;

  auto r8d = Dyninst::x86_64::r8d;
  auto rbp = Dyninst::x86_64::rbp;

  // clang-format off
  return {
    { // mov dword ptr [rbp - 0x3c], r8d
      {0x44, 0x89, 0x45, 0xc4},
      rs_t{rbp, r8d},
      rs_t{}
    }
  };
  // clang-format on
}
