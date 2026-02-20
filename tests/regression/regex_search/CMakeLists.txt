include_guard(GLOBAL)

include(DyninstBinaryTests)

if(NOT BUILD_RTLIB_32)
  message(FATAL "regex_search tests require the 32-bit runtime (BUILD_RTLIB_32=ON)")
endif()

add_library(regex_search_tests INTERFACE)
target_sources(regex_search_tests INTERFACE regex_search.h)
target_include_directories(regex_search_tests INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

set(_base "${DYNINST_TEST_BINARIES_DIR}/regex_search")

list(APPEND DYNINST_REGEX_TEST_FILES "${_base}/libregex_search.x86_64.so")
list(APPEND DYNINST_REGEX_TEST_FILES "${_base}/libregex_search.x86.so")

unset(_base)


add_subdirectory(dyninstAPI/addressSpace)
#add_subdirectory(dyninstAPI/BPatch_image)
add_subdirectory(dyninstAPI/image)
#add_subdirectory(dyninstAPI/mapped_object)
