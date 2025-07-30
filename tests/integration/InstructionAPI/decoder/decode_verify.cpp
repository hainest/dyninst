#include "decode_verify.h"

#include <iostream>

namespace Dyninst { namespace InstructionAPI {

  void show_diff_set(Dyninst::register_set const&, Dyninst::register_set const&);

  bool verify_read_write_sets(Instruction i, register_set const& expected_read, register_set const& expected_written) {
    auto convert = [](std::set<RegisterAST::Ptr> const& regs) {
      register_set rs;
      for(auto r : regs) {
        rs.insert(r->getID());
      }
      return rs;
    };
    auto actual_read = [&i, &convert]() {
      std::set<RegisterAST::Ptr> regs;
      i.getReadSet(regs);
      return convert(regs);
    }();
    auto actual_written = [&i, &convert]() {
      std::set<RegisterAST::Ptr> regs;
      i.getWriteSet(regs);
      return convert(regs);
    }();

    bool failed = false;

    if(actual_read != expected_read) {
      std::cerr << "Mismatched READ set for '" << i.format() << "'\n";
      show_diff_set(actual_read, expected_read);
      std::cerr << '\n';
      failed = true;
    }

    if(actual_written != expected_written) {
      std::cerr << "Mismatched WRITE set for '" << i.format() << "'\n";
      show_diff_set(actual_written, expected_written);
      std::cerr << '\n';
      failed = true;
    }

    return !failed;
  }

  void show_diff_set(Dyninst::register_set const& actual, Dyninst::register_set const& expected) {
    auto intersection = actual & expected;
    {
      auto unused = actual - intersection;
      if(!unused.is_empty()) {
        std::cerr << "Used but not expected: { ";
        for(auto const& r : unused) {
          std::cerr << r.name() << ", ";
        }
        std::cerr << "}\n";
      }
    }
    {
      auto unused = expected - intersection;
      if(!unused.is_empty()) {
        std::cerr << "Expected but not used: { ";
        for(auto const& r : unused) {
          std::cerr << r.name() << ", ";
        }
        std::cerr << "}\n";
      }
    }
  }

}}
