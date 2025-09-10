#include "Architecture.h"
#include "../memory_tests.h"
#include "../register_tests.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  Intel 64 and IA-32 Architectures Software Developer’s Manual (SDM)
 *  June 2025
 *  5.1.7 Control Transfer Instructions
 *
 *  These tests only cover system-level control instructions.
 */

namespace di = Dyninst::InstructionAPI;

struct sysctl_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<sysctl_test> make_tests(Dyninst::Architecture arch);
std::vector<sysctl_test> make_tests32();
static bool run(Dyninst::Architecture, std::vector<sysctl_test> const&);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }
  if(!run(Dyninst::Arch_x86_64, make_tests32())) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<sysctl_test> const& tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'sysctl' in " << sarch << " mode\n";
  for(auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.opcode.data(), t.opcode.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.regs)) {
      failed = true;
    }
    if(!di::verify(insn, t.mem)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<sysctl_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

//  auto ax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;
//  auto bx = is_64 ? Dyninst::x86_64::rbx : Dyninst::x86::ebx;
//  auto cx = is_64 ? Dyninst::x86_64::rcx : Dyninst::x86::ecx;
//  auto al = is_64 ? Dyninst::x86_64::al : Dyninst::x86::al;
//  auto bp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;
//  auto dl = is_64 ? Dyninst::x86_64::dl : Dyninst::x86::dl;
//  auto si = is_64 ? Dyninst::x86_64::rsi : Dyninst::x86::esi;

  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto ac = is_64 ? Dyninst::x86_64::ac : Dyninst::x86::ac;
//  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
//  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
//  auto df = is_64 ? Dyninst::x86_64::df : Dyninst::x86::df;
  auto flagc = is_64 ? Dyninst::x86_64::flagc : Dyninst::x86::flagc;
  auto flagd = is_64 ? Dyninst::x86_64::flagd : Dyninst::x86::flagd;
  auto if_ = is_64 ? Dyninst::x86_64::if_ : Dyninst::x86::if_;
  auto nt = is_64 ? Dyninst::x86_64::nt_ : Dyninst::x86::nt_;
//  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
//  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto rf = is_64 ? Dyninst::x86_64::rf : Dyninst::x86::rf;
//  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto tf = is_64 ? Dyninst::x86_64::tf : Dyninst::x86::tf;
  auto vif = is_64 ? Dyninst::x86_64::vif : Dyninst::x86::vif;
  auto vm = is_64 ? Dyninst::x86_64::vm : Dyninst::x86::vm;
//  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;

//  auto gs = is_64 ? Dyninst::x86_64::gs : Dyninst::x86::gs;
//  auto ds = is_64 ? Dyninst::x86_64::ds : Dyninst::x86::ds;
//
//  auto ax16 = is_64 ? Dyninst::x86_64::ax : Dyninst::x86::ax;
//  auto ax32 = is_64 ? Dyninst::x86_64::eax : Dyninst::x86::eax;
//  auto bp32 = is_64 ? Dyninst::x86_64::ebp : Dyninst::x86::ebp;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);
  auto ip = Dyninst::MachRegister::getPC(arch);

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // int3
      {0xcc},
      di::register_rw_test{
        reg_set{flags, sp, flagc, flagd, vm},
        reg_set{flags, ip, sp, tf, if_, nt, rf, vm, ac, vif}
      },
      di::mem_test{}
    },
    { // int 80
      {0xcd, 0x80},
      di::register_rw_test{
        reg_set{flags, sp, flagc, flagd, vm},
        reg_set{flags, ip, sp, tf, if_, nt, rf, vm, ac, vif}
      },
      di::mem_test{}
    },
    { // int1
      {0xf1},
      di::register_rw_test{
        reg_set{sp},
        reg_set{ip, sp}
      },
      di::mem_test{}
    },

//    { // iretd
//      !readsMemory, !writesMemory,
//      di::register_rw_test {
//        {0xcf},
//        rs_t{},
//        rs_t{}
//      }
//    },
//    { // iret
//     {0xcf},
//     !hasCFT,
//     {}
//    },
  };
  // clang-format on
}

std::vector<sysctl_test> make_tests64() {
  auto sp = Dyninst::MachRegister::getStackPointer(Dyninst::Arch_x86_64);

  auto rdi = Dyninst::x86_64::rdi;
  auto es = Dyninst::x86_64::es;

  using reg_set = Dyninst::register_set;

  constexpr bool readsMemory = true;
  constexpr bool writesMemory = true;

  // clang-format off
  return {
    { // iretq
      {0x40, 0xcf}, // REX.W
      di::register_rw_test {
        reg_set{},
        reg_set{sp}
      },
      di::mem_test{
        !readsMemory, writesMemory,
        di::register_rw_test{
          reg_set{},
          reg_set({rdi, es})
        }
      }
    },
  };
  // clang-format on
}

std::vector<sysctl_test> make_tests32() {
  auto sp = Dyninst::MachRegister::getStackPointer(Dyninst::Arch_x86);
  auto ip = Dyninst::MachRegister::getPC(Dyninst::Arch_x86);

  auto flags = Dyninst::x86::flags;
  auto ac = Dyninst::x86::ac;
  auto flagc = Dyninst::x86::flagc;
  auto flagd = Dyninst::x86::flagd;
  auto if_ = Dyninst::x86::if_;
  auto nt = Dyninst::x86::nt_;
  auto of = Dyninst::x86::of;
  auto rf = Dyninst::x86::rf;
  auto tf = Dyninst::x86::tf;
  auto vm = Dyninst::x86::vm;

  using reg_set = Dyninst::register_set;

  constexpr bool readsMemory = true;
  constexpr bool writesMemory = true;

  // clang-format off
  return {
    { // into
      {0xce},
      reg_set{flags, sp, flagc, flagd, of, vm},
      reg_set{flags, ip, sp, tf, if_, nt, rf, vm, ac}
    },

  };
  // clang-format on
}
