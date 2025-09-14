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
 *
 *  These tests only cover stack-manipulation instructions.
 */

namespace di = Dyninst::InstructionAPI;

struct call_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<call_test> make_tests(Dyninst::Architecture arch);
static bool run(Dyninst::Architecture, std::vector<call_test> const&);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<call_test> const& tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'stack' in " << sarch << " mode\n";
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

std::vector<call_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto eax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // push eax
      {0x50},
      di::register_rw_test{
        reg_set {sp, eax},
        reg_set {sp}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      }
    },
    { // pop eax
      {0x58},
      di::register_rw_test{
        reg_set {sp},
        reg_set {sp, eax}
      },
      di::mem_test { // pop eax
        reads_memory, !writes_memory,
        di::register_rw_test {
          reg_set{sp},
          reg_set{}
        }
      }
    }

    // pusha/popa, pushf/popf
  };
  // clang-format on
}
