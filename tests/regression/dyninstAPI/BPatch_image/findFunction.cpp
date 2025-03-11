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
    for(const auto &t : function_tests) {
      std::vector<BPatch_function *> funcs{};
      constexpr bool show_error = true;
      constexpr bool regex_case_sensitive = false;
      constexpr bool include_uninstrumentable = false;
      auto res = img->findFunction(t.input().c_str(), funcs, show_error,
                                   regex_case_sensitive, include_uninstrumentable);
      if(!res && !t.should_fail()) {
        std::cerr << "Failed to find function '" << t.input() << "'\n";
        failed = true;
      }
    }
    // clang-format on
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
