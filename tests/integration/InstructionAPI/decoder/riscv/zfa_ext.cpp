#include "Architecture.h"
#include "InstructionDecoder.h"
#include "register_tests.h"
#include "opcode_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/riscv64_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  The RISC-V Instruction Set Manual Volume I: Unprivileged Architecture
 *  Chapter 26. "Zfa" Extension for Additional Floating-Point Instructions, Version 1.0
 */

namespace di = Dyninst::InstructionAPI;

struct zfa_ext_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  double imm;
};

static std::vector<zfa_ext_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<zfa_ext_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<zfa_ext_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'zfa_ext' in " << sarch << " mode\n";
  for (auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.rawBytes.data(), t.rawBytes.size(), arch);
    auto insn = d.decode();
    if (!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if (!di::verify(insn, t.regs)) {
      failed = true;
    }
    if (!di::verify(insn, t.opcodes)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<zfa_ext_tests> make_tests64() {
  auto ft1 = Dyninst::riscv64::ft1;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // fli.s ft1, -1.0
      {0xd3, 0x00, 0x10, 0xf0},
      di::opcode_test{riscv64_op_mul, "fli.s", "fli.s ft1, -1.0"},
      di::register_rw_test{ reg_set{}, reg_set{ft1} },
    },
  };
  // clang-format on
}
