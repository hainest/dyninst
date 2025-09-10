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

struct strings_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<strings_test> make_tests32();
static std::vector<strings_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<strings_test> const&);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests32());

  if(!run(Dyninst::Arch_x86_64, make_tests64())) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<strings_test> const& tests) {
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

std::vector<strings_test> make_tests32() {
  auto al = Dyninst::x86::al;
  auto eax = Dyninst::x86::eax;
  auto edi = Dyninst::x86::edi;

  auto es = Dyninst::x86::es;

  auto df = Dyninst::x86::df;
  auto eflags = Dyninst::x86::flags;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { //  stosb
      {0xaa},
      di::register_rw_test{
        reg_set({al, edi, es, df, eflags}),
        reg_set{edi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set({edi, es})
        }
      }
    },
    { //  stosw/stosd
      {0xab},
      di::register_rw_test{
        reg_set({eax, edi, es, df, eflags}),
        reg_set{edi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set({edi, es})
        }
      }
    },
//    { // rep lodsb al, byte ptr [esi]
//      {0xf3, 0xac},
//      di::register_rw_test{
//        reg_set{si, ds, flags, cx, df},
//        reg_set{al, si, cx}
//      },
//      di::mem_test{
//        !reads_memory, writes_memory,
//        di::register_rw_test{
//          reg_set{},
//          reg_set({edi, es})
//        }
//      }
//    }
//    { // stosb
//      {0xaa},
//      rs_t{edi, es, al, flags, df},
//      rs_t{edi},
//    },
//    { // stosd(w)
//      {0xab},
//      rs_t{edi, es, al, flags, df},
//      rs_t{edi},
//    },
//    { // scasb
//      {0xae},
//      rs_t{edi, es, al, flags, df},
//      rs_t{flags, edi, of, sf, zf, af, pf, cf},
//    },
//    { // scasd
//      {0xaf},
//      rs_t{edi, es, eax, flags, df},
//      rs_t{flags, edi, of, sf, zf, af, pf, cf},  // always writes edi
//    },
  // clang-format on
  };
}

std::vector<strings_test> make_tests64() {
  auto al = Dyninst::x86_64::al;
  auto eax = Dyninst::x86_64::eax;

  auto edi = Dyninst::x86_64::edi;
  auto rdi = Dyninst::x86_64::rdi;
  auto es = Dyninst::x86_64::es;

  auto df = Dyninst::x86_64::df;
  auto eflags = Dyninst::x86_64::flags;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { //  stosb
      {0xaa},
      di::register_rw_test{
        reg_set({al, rdi, df, es, eflags}),
        reg_set{rdi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set({rdi})
        }
      }
    },
    { //  stosw/stosd
      {0xab},
      di::register_rw_test{
        reg_set({eax, rdi, es, df, eflags}),
        reg_set{rdi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set({rdi})
        }
      }
    },
    { //  stosq
      {0x67, 0xab},
      di::register_rw_test{
        // Docs say 'Store RAX at address RDI or EDI.', but xed et. al use eax
        // BUG: Capstone incorrectly reads from rdi
        reg_set({eax, rdi, edi, es, df, eflags}),
        reg_set{rdi}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set({edi})
        }
      },
//      { // scasq
//        {0x40, 0xaf},
//        rs_t{rdi, es, eax, eax32, rflags, df},
//        rs_t{rflags, edi, of, sf, zf, af, pf, cf},  // always writes edi
//      },
//      { // stosq
//        {0x40, 0xab},
//        rs_t{rdi, es, eax32, rflags, df},
//        rs_t{rdi},                                  // XXX: Should be edi
//      },
    },
//    { // rep lodsb al, byte ptr [esi]
//     {0xf3, 0xac},
//     rs_t{si, ds, flags, cx, df},
//     rs_t{al, si, cx}
//    }
  // clang-format on
  };
}

//MOVS/MOVSB Move string/Move byte string.
//MOVS/MOVSW Move string/Move word string.
//MOVS/MOVSDMove string/Move doubleword string.
//CMPS/CMPSBCompare string/Compare byte string.
//CMPS/CMPSWCompare string/Compare word string.
//CMPS/CMPSDCompare string/Compare doubleword string.
//SCAS/SCASBScan string/Scan byte string.
//SCAS/SCASWScan string/Scan word string.
//SCAS/SCASDScan string/Scan doubleword string.
//LODS/LODSBLoad string/Load byte string.
//LODS/LODSWLoad string/Load word string.
//LODS/LODSDLoad string/Load doubleword string.
//REPRepeat while ECX not zero.
//REPE/REPZRepeat while equal/Repeat while zero.
//REPNE/REPNZRepeat while not equal/Repeat while not zero.
