#include "addressSpace.h"
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "mapped_object.h"
#include "test_binaries.h"

#include <array>
#include <iostream>

int main(int argc, char **argv) {
  if(argc < 2) {
    std::cerr << "Usage: " << argv[0] << " file [file ...]\n";
    return EXIT_FAILURE;
  }

  bool failed = false;

  for(int idx = 1; idx < argc; idx++) {
    auto const *cur_library_filename = argv[idx];

    std::cout << "Parsing " << cur_library_filename << "\n";

    constexpr bool resolve_deps = true;
    BPatch_binaryEdit *source = BPatch::bpatch->openBinary(cur_library_filename, resolve_deps);

    if(!source) {
      std::cerr << "Failed to open file '" << cur_library_filename << "'\n";
      failed = true;
      continue;
    }

    std::vector<AddressSpace *> address_spaces{};
    source->getAS(address_spaces);

    if(address_spaces.empty()) {
      std::cerr << "No address spaces in '" << cur_library_filename << "'\n";
      failed = true;
      continue;
    }

    for(auto *as : address_spaces) {
      for(auto const &t : module_tests) {
        constexpr bool is_regex = true;
        if(!as->findModule(t.input(), is_regex) && !t.should_fail()) {
          std::cerr << "Failed to find module '" << t.input() << "'\n";
          failed = true;
        }
      }
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
