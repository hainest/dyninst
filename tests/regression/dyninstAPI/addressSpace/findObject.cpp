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

    const bool has_objects = [&]() {
      int cnt = 0;
      for(auto *as : address_spaces) {
        if(!as->mappedObjects().empty()) {
          cnt++;
        }
      }
      // There is always an object for the binary, itself
      return cnt > 1;
    }();

    bool found_any = false;
    bool found_self = false;

    for(auto *as : address_spaces) {
      for(auto const &t : dependency_libraries) {
        constexpr bool is_regex = true;
        if(as->findObject(t, is_regex)) {
          found_any = true;
          break; // if at least one is found, then the test passes
        }
      }
      // Must find ourself
      constexpr bool is_regex = false;
      if(as->findObject(cur_library_filename, is_regex)) {
        found_self = true;
      }
    }

    if(!found_self || (has_objects && !found_any)) {
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
