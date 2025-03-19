#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "test_binaries.h"

#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char **argv) {
  if(argc < 2) {
    std::cerr << "Usage: " << argv[0] << " file [file ...]\n";
    return EXIT_FAILURE;
  }

  bool failed = false;

  for(int idx = 1; idx < argc; idx++) {
    const auto *cur_library_filename = argv[idx];

    std::cout << "Parsing " << cur_library_filename << "\n";

    constexpr bool resolve_deps = true;
    BPatch_binaryEdit *source = BPatch::bpatch->openBinary(cur_library_filename, resolve_deps);

    if(!source) {
      std::cerr << "Failed to open file '" << cur_library_filename << "'\n";
      failed = true;
      continue;
    }

    BPatch_image *img = source->getImage();

    // clang-format off
    for(const auto &t : module_tests) {
      constexpr bool substring_match = true;
      auto *res = img->findModule(t.input().c_str(), substring_match);
      if(!res && !t.should_fail()) {
        std::cerr << "Failed to find module '" << t.input() << "'\n";
        failed = true;
      }
    }
    // clang-format on
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
