include_guard(GLOBAL)

# Need 3.16 for COMMAND_EXPAND_LISTS
cmake_minimum_required(VERSION 3.16)

# Use same warning flags as used to build Dyninst
set(UNIT_TESTS_WARNING_FLAGS ${SUPPORTED_CXX_WARNING_FLAGS})

string(TOUPPER ${DYNINST_ENABLE_TESTS} _test_flag)

if(_test_flag STREQUAL "REGRESSION" OR _test_flag STREQUAL "ALL")
  add_subdirectory(regression)

  # Implicitly enable integration tests
  set(_test_flag "INTEGRATION")
endif()

if(_test_flag STREQUAL "INTEGRATION")
  add_subdirectory(integration)

  # Implicitly enable unit tests
  set(_test_flag "UNIT")
endif()

if(_test_flag OR _test_flag STREQUAL "UNIT")
  add_subdirectory(unit)
endif()

unset(_test_flag)
