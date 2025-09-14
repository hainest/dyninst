#include "Architecture.h"
#include "../cft_tests.h"
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

struct branch_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
  di::cft_test cft;
};

static std::vector<branch_test> make_tests(Dyninst::Architecture arch);
static bool run(Dyninst::Architecture, std::vector<branch_test> const&);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<branch_test> const& tests) {
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
    if(!di::verify(insn, t.cft)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<branch_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;

  auto ip = Dyninst::MachRegister::getPC(arch);

  constexpr auto hasCFT = true;
  constexpr auto isCall = true;
  constexpr auto isConditional = true;
  constexpr auto isIndirect = true;
  constexpr auto isFallthrough = true;
  constexpr auto isBranch = true;
  constexpr auto isReturn = true;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // je 0x12
      {0x74, 0x10},
      di::register_rw_test{
        reg_set{zf, sf, cf, pf, of, ip, flags},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        hasCFT,
        {!isCall, isConditional, !isIndirect, isFallthrough, isBranch, !isReturn}
      }
    },
//      nonOperandRegisterWrites.insert(make_pair(e_loop, thePC));
//      nonOperandRegisterWrites.insert(make_pair(e_loope, thePC));
//      nonOperandRegisterWrites.insert(make_pair(e_loopne, thePC));
  };
  // clang-format on
}
