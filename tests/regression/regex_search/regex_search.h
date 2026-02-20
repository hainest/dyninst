#ifndef DYNINST_TESTS_REGRESSION_REGEX_SEARCH_H
#define DYNINST_TESTS_REGRESSION_REGEX_SEARCH_H

#include <array>
#include <string>

class test_t {
  std::string input_{};
  bool should_fail_{false};

public:
  explicit test_t(std::string input, bool should_fail = false) noexcept
      : input_{std::move(input)}, should_fail_{should_fail} {}

  bool should_fail() const {
    return should_fail_;
  }

  std::string const &input() const {
    return input_;
  }
};

namespace {

  constexpr auto SHOULD_FAIL = true;

  // clang-format off

  /* mapped_object::findModule
   * 
   *  1. is case-insensitive when doing a regex match
   *  2. allows for the extended POSIX '*' and '?' characters
   */
  const std::array<test_t, 8> module_tests = {{
    test_t(R"(regex_search\.cpp)"),
    test_t("regex_search*"),
    test_t(R"(REGEX_SEARCH\.CPP)"),
    test_t("REGEX_SEARCH"),
    test_t(R"(.*regex_SEARCH.*)"),
    test_t(R"(regex_SE.R.+\.cpp)"),
    test_t(R"(regex_search\..*)"),
    test_t(R"([[:digit:]])", SHOULD_FAIL)
  }};

  const std::array<test_t, 13> function_tests = {{
    test_t("foo"),
    test_t("FOO1"),
    test_t(R"(foo.*)"),
    test_t("foo3"),
    test_t(R"(foo[[:digit:]])"),
    test_t(R"(foo[[:digit:]]+)"),
    test_t(R"(FOO)"),
    test_t("test1"),
    test_t(R"(Dyninst::RegexTests::test1)"),
    test_t(R"(RegexTests::test1)"),
    test_t(R"(dyninst::regextests::test1)"),
    test_t("_ZN7Dyninst11RegexSearch5test1Ev"),
    test_t(R"([[:digit:]])", SHOULD_FAIL)
  }};

  /* AddressSpace::findObject
   * 
   *  1. is case-insensitive when doing a regex match
   *  2. allows for the extended POSIX '*' and '?' characters
   */
  const std::array<std::string, 5> dependency_libraries = {{
    R"(libc\.so.*)",
    R"(libc.*)",
    R"(libc\.so.*\,*[[:digit:]]+)",
    R"(ld-linux-.*\.so.*)",
    R"(ld-linux.*)"
  }};

  // clang-format on

}

#endif
