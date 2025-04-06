#ifndef DYNINST_TESTS_REGRESSION_BINARIES_TEST_BINARIES_H
#define DYNINST_TESTS_REGRESSION_BINARIES_TEST_BINARIES_H

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
  const std::array<test_t, 8> module_tests = {{
    test_t(R"(regression_test_binaries\.cpp)"),
    test_t("regression_test_binaries"),
    test_t(R"(REGRESSION_TEST_BINARIES\.CPP)"),
    test_t("REGRESSION_TEST"),
    test_t(R"(.*regression_TEST.*)"),
    test_t(R"(regression_TE.T_bin.+\.cpp)"),
    test_t(R"(regression_test_binaries\..*)"),
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
    test_t(R"(Dyninst::RegressionTests::test1)"),
    test_t(R"(RegressionTests::test1)"),
    test_t(R"(dyninst::regressiontests::test1)"),
    test_t("_ZN7Dyninst15RegressionTests5test1Ev"),
    test_t(R"([[:digit:]])", SHOULD_FAIL)
  }};

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
