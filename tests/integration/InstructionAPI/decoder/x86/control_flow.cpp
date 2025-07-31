#include "Architecture.h"
#include "decode_verify.h"
#include "InstructionDecoder.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

struct cftExpected {
  bool isCall;
  bool isConditional;
  bool isIndirect;
  bool isFallthrough;
};

struct cft_test_t {
  di::rw_test_t insn;
  cftExpected cft;
};

bool run_tests(Dyninst::Architecture, std::vector<cft_test_t> const &);
std::vector<cft_test_t> make_tests(Dyninst::Architecture);

int main() {
  bool ok = run_tests(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));
  if(!run_tests(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run_tests(Dyninst::Architecture arch, std::vector<cft_test_t> const &tests) {
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

    if(!verify_read_write_sets(insn, t.insn.read, t.insn.written)) {
      failed = true;
      continue;
    }

    const auto num_cft = std::distance(insn.cft_begin(), insn.cft_end());
    if(!num_cft) {
      std::cerr << sarch << ": No control flow targets found for '" << insn.format() << "'\n";
      failed = true;
      continue;
    }

    auto all_cfts = boost::make_iterator_range(insn.cft_begin(), insn.cft_end());
    for(auto const &actual : all_cfts) {
      auto const &expected = t.cft;
      if(actual.isCall != expected.isCall) {
        std::cerr << std::boolalpha << "Expected isCall = " << expected.isCall << ", got '" << actual.isCall << "'\n";
        failed = true;
        continue;
      }
      if(actual.isIndirect != expected.isIndirect) {
        std::cerr << std::boolalpha << "Expected isIndirect = " << expected.isIndirect << ", got '" << actual.isIndirect
                  << "'\n";
        failed = true;
        continue;
      }
      if(actual.isConditional != expected.isConditional) {
        std::cerr << std::boolalpha << "Expected isConditional = " << expected.isConditional << ", got '"
                  << actual.isConditional << "'\n";
        failed = true;
        continue;
      }
      if(actual.isFallthrough != expected.isFallthrough) {
        std::cerr << std::boolalpha << "Expected isFallthrough = " << expected.isFallthrough << ", got '"
                  << actual.isFallthrough << "'\n";
        failed = true;
        continue;
      }
    }
  }
  return !failed;
}

std::vector<cft_test_t> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto eax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;
  auto ecx = is_64 ? Dyninst::x86_64::rcx : Dyninst::x86::ecx;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);
  auto ip = Dyninst::MachRegister::getPC(arch);

  using rs_t = Dyninst::register_set;

  constexpr auto is_call = true;
  constexpr auto is_conditional = true;
  constexpr auto is_indirect = true;
  constexpr auto is_fallthrough = true;
  //  constexpr auto is_branch = true;
  //  constexpr auto is_return = true;

  // clang-format off
  return {
    { // call qword ptr [rcx+rax*8-0x21524111]
      {
        {0xff, 0x94, 0xc1, 0xef, 0xbe, 0xad, 0xde},
        rs_t{ecx, eax, ip, sp},
        rs_t{ip, sp}
      },
      {is_call, !is_conditional, is_indirect, !is_fallthrough}
    }
  };
  // clang-format on
}
