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
 *  5.1.8 String Instructions
 */

namespace di = Dyninst::InstructionAPI;

struct sse_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<sse_test> make_tests();
static bool run(Dyninst::Architecture, std::vector<sse_test> const&);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests());

  if(!run(Dyninst::Arch_x86_64, make_tests())) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<sse_test> const& tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'strings' in " << sarch << " mode\n";
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

std::vector<sse_test> make_tests() {
  auto xmm0 = Dyninst::x86::xmm0;
  auto xmm1 = Dyninst::x86::xmm1;
  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // movddup xmm0, xmm0
      {0xf2, 0x0f, 0x12, 0xc0},
      di::register_rw_test{
        reg_set{xmm0},
        reg_set{xmm0}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{}
      }
    },
    { // haddpd xmm1, xmm1
      {0x66, 0x0f, 0x7c, 0xc9},
      di::register_rw_test{
        reg_set{xmm1},
        reg_set{xmm1}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{}
      }
    },
  // clang-format on
  };
}
