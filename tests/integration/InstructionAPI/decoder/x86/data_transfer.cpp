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

struct data_transfer_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<data_transfer_test> make_tests(Dyninst::Architecture arch);
std::vector<data_transfer_test> make_tests32();
static bool run(Dyninst::Architecture, std::vector<data_transfer_test> const&);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<data_transfer_test> const& tests) {
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

std::vector<data_transfer_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto ax16 = is_64 ? Dyninst::x86_64::ax : Dyninst::x86::ax;
  auto ax32 = is_64 ? Dyninst::x86_64::eax : Dyninst::x86::eax;
  auto ebx = is_64 ? Dyninst::x86_64::rbx : Dyninst::x86::ebx;
  auto ebp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;
  auto bp32 = is_64 ? Dyninst::x86_64::bp : Dyninst::x86::bp;
  auto dl = is_64 ? Dyninst::x86_64::dl : Dyninst::x86::dl;
  auto gs = is_64 ? Dyninst::x86_64::gs : Dyninst::x86::gs;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // mov dword ptr [ebp - 4], 1
      {0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00},
      di::register_rw_test {
        reg_set{ebp},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
    },
    { // mov byte ptr [ebp - 0x34], dl
      {0x88, 0x55, 0xcc},
      di::register_rw_test {
        reg_set{ebp, dl},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
    },
    { // lea eax, [ebx - 0xe8]
      {0x8d, 0x83, 0x18, 0xff, 0xff, 0xff},
      di::register_rw_test {
        reg_set{ebx},
        reg_set{ax32}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
    },
    { // mov ax, gs
      {0x66, 0x8c, 0xe8},
      di::register_rw_test {
        reg_set{gs},
        reg_set{ax16}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
    },
    { // mov eax, ebp
      {0x89, 0xe8},
      di::register_rw_test {
        reg_set{bp32},
        reg_set{ax32}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
    },
  };
  // clang-format on
}

std::vector<data_transfer_test> make_tests32() {
  auto rbp = Dyninst::x86_64::rdi;
  auto r8d = Dyninst::x86_64::es;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // mov dword ptr [rbp - 0x3c], r8d
      {0x44, 0x89, 0x45, 0xc4},
      di::register_rw_test {
        reg_set{rbp, r8d},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
    },
  };
  // clang-format on
}
