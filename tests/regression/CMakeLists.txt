include_guard(GLOBAL)

message(STATUS "Enabling regression tests")

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/tests/regression")

add_subdirectory(regex_search)
add_subdirectory(symtabAPI)
