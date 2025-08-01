#include "Architecture.h"
#include "decode_verify.h"
#include "InstructionDecoder.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

struct mem_test_t {
  di::rw_test_t insn;
};

bool run_tests(Dyninst::Architecture, std::vector<mem_test_t> const &);
std::vector<mem_test_t> make_tests(Dyninst::Architecture);

int main() {
  bool ok = run_tests(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));
  if(!run_tests(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run_tests(Dyninst::Architecture arch, std::vector<mem_test_t> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = (arch == Dyninst::Arch_x86) ? "x86" : "x86_64";
  for(auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.insn.opcode.data(), t.insn.opcode.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    auto const ok = verify_read_write_sets(insn, t.insn.read, t.insn.written);
    if(!ok) {
      std::cerr << sarch << " test " << test_id << " failed\n";
      failed = true;
    }
  }
  return !failed;
}

std::vector<mem_test_t> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto ax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;
  auto cx = is_64 ? Dyninst::x86_64::rcx : Dyninst::x86::ecx;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);
  auto ip = Dyninst::MachRegister::getPC(arch);

  using rs_t = Dyninst::register_set;

  // clang-format off
  return {
    { // call [8*EAX + ECX + 0xDEADBEEF]
      di::rw_test_t {
        {0xff, 0x94, 0xc1, 0xef, 0xbe, 0xad, 0xde},
        rs_t{ax, cx, sp, ip},
        rs_t{sp, ip}
      }
    },
    { // push eax
      di::rw_test_t {
        {0x50},
        rs_t{ax, sp},
        rs_t{sp}
      }
    },
    { // pop eax
      di::rw_test_t {
        {0x58},
        rs_t{sp},
        rs_t{sp, ax}
      }
    }
  };
  // clang-format on
}
