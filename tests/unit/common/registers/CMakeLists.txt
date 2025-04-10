include_guard(GLOBAL)

add_executable(register_set register_set.cpp)
target_compile_options(register_set PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(register_set PRIVATE common)
add_test(NAME common_register_set COMMAND register_set)
set_tests_properties(common_register_set PROPERTIES LABELS "unit")
