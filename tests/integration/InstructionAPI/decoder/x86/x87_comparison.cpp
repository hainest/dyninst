#include "decode_verify.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <boost/smart_ptr/make_shared.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

std::vector<di::rw_test_t> make_tests(Dyninst::Architecture);
bool run_tests(Dyninst::Architecture);

int main() {
  auto const run32 = run_tests(Dyninst::Arch_x86);
  auto const run64 = run_tests(Dyninst::Arch_x86_64);
  return !(run32 && run64) ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run_tests(Dyninst::Architecture arch) {
  bool failed = false;
  int test_id = 0;
  auto sarch = (arch == Dyninst::Arch_x86) ? "x86" : "x86_64";
  for(auto const &t : make_tests(arch)) {
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
      failed = true;
    }
  }
  return !failed;
}

std::vector<di::rw_test_t> make_tests(Dyninst::Architecture arch) {
  bool const is_64 = (arch == Dyninst::Arch_x86_64);

  auto st0 = is_64 ? Dyninst::x86_64::st0 : Dyninst::x86::st0;
  auto st1 = is_64 ? Dyninst::x86_64::st1 : Dyninst::x86::st1;
  auto st2 = is_64 ? Dyninst::x86_64::st2 : Dyninst::x86::st2;
  auto ebp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;

  auto eflags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;

  auto fsw = is_64 ? Dyninst::x86_64::fsw : Dyninst::x86::fsw;
  auto cr0 = is_64 ? Dyninst::x86_64::cr0 : Dyninst::x86::cr0;
  auto cr1 = is_64 ? Dyninst::x86_64::cr1 : Dyninst::x86::cr1;
  auto cr2 = is_64 ? Dyninst::x86_64::cr2 : Dyninst::x86::cr2;
  auto cr3 = is_64 ? Dyninst::x86_64::cr3 : Dyninst::x86::cr3;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
      /* ordered comparisons */

    {//  fcom dword ptr [ebp]
      {0xd8, 0x55, 0x00},
      reg_set({st0, ebp}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcom qword ptr [ebp]
      {0xdc, 0x55, 0x00},
      reg_set({st0, ebp}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcom st0
      {0xd8, 0xd0},
      reg_set({st0}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcom st1
      {0xd8, 0xd1},
      reg_set({st0, st1}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcom st2
      {0xd8, 0xd2},
      reg_set({st0, st2}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcomp dword ptr [ebp]
      {0xd8, 0x5d, 0x00},
      reg_set({st0, ebp}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcomp qword ptr [ebp]
      {0xdc, 0x5d, 0x00},
      reg_set({st0, ebp}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcomp st0
      {0xd8, 0xd8},
      reg_set({st0}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcomp st1
      {0xd8, 0xd9},
      reg_set({st0, st1}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcomp st2
      {0xd8, 0xda},
      reg_set({st0, st2}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fcompp
      {0xde, 0xd9},
      reg_set({st0, st1}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },

    /* compare and set eflags */

    { //  fcomi st0, st0
      {0xdb, 0xf0},
      reg_set({st0}),
      reg_set({eflags, cf, pf, af, zf, sf, of})
    },
    { //  fcomip st0, st0
      {0xdf, 0xf0},
      reg_set({st0}),
      reg_set({eflags, cf, pf, af, zf, sf, of})
    },
    { //  fcomip st0, st1
      {0xdf, 0xf1},
      reg_set({st0, st1}),
      reg_set({eflags, cf, pf, af, zf, sf, of})
    },
    { //  fucomi st0, st0
      {0xdb, 0xe8},
      reg_set({st0}),
      reg_set({eflags, cf, pf, af, zf, sf, of})
    },
    { //  fucomi st0, st1
      {0xdb, 0xe9},
      reg_set({st0, st1}),
      reg_set({eflags, cf, pf, af, zf, sf, of})
    },
    { //  fucomip st0, st0
      {0xdf, 0xe8},
      reg_set({st0}),
      reg_set({eflags, cf, pf, af, zf, sf, of})
    },
    { //  fucomip st0, st1
      {0xdf, 0xe9},
      reg_set({st0, st1}),
      reg_set({eflags, cf, pf, af, zf, sf, of})
    },

    /* unordered comparisons */

    { //  fucom st0
      {0xdd, 0xe0},
      reg_set({st0}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fucom st1 (aka, 'fucom')
      {0xdd, 0xe1},
      reg_set({st0, st1}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fucom st2
      {0xdd, 0xe2},
      reg_set({st0, st2}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fucomp
      {0xdd, 0xe8},
      reg_set({st0}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fucompp
      {0xdd, 0xe9},
      reg_set({st0, st1}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    },
    { //  fucomp st2
      {0xdd, 0xea},
      reg_set({st0, st2}),
      reg_set({fsw, cr0, cr1, cr2, cr3})
    }
  };
  // clang-format on
}
