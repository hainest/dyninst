#include "dyninstAPI/src/image.h"
#include "test_binaries.h"

#include <array>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
  if(argc < 2) {
    std::cerr << "Usage: " << argv[0] << " file [file ...]\n";
    return EXIT_FAILURE;
  }

  bool failed = false;

  for(int idx = 1; idx < argc; idx++) {
    auto const* cur_library_filename = argv[idx];

    std::cout << "Parsing " << cur_library_filename << "\n";

    auto fd = fileDescriptor(cur_library_filename, Dyninst::ADDR_NULL, Dyninst::ADDR_NULL);

    constexpr bool parse_gaps = false;
    auto *img = image::parseImage(fd, BPatch_normalMode, parse_gaps);

    if(!img) {
      std::cerr << "Failed to open file '" << cur_library_filename << "'\n";
      failed = true;
      continue;
    }

    {
      std::vector<pdmodule*> modules{};
      img->getModules(modules);
      if(modules.empty()) {
        std::cerr << "No modules in '" << cur_library_filename << "'\n";
        failed = true;
        continue;
      }
    }

    constexpr bool is_regex = true;
    for(auto const& t : module_tests) {
      if(!img->findModule(t.input(), is_regex) && !t.should_fail()) {
        std::cerr << "Failed to find module '" << t.input() << "'\n";
        failed = true;
      }
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
