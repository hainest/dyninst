include_guard(GLOBAL)

list(APPEND _rt_libs "DYNINSTAPI_RT_LIB=${CMAKE_BINARY_DIR}/dyninstAPI_RT/libdyninstAPI_RT.so")
list(APPEND _rt_libs "DYNINSTAPI_RT_LIB=${CMAKE_BINARY_DIR}/dyninstAPI_RT/libdyninstAPI_RT_m32.so")

macro(_mk_test _name)
  set(_targ "regex_search_dyninstapi_addressSpace_${_name}")
  
  add_executable(${_targ} ${_name}.cpp)
  target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_compile_definitions(${_targ} PRIVATE ${DYNINST_PLATFORM_CAPABILITIES})
  target_link_libraries(${_targ} PRIVATE regex_search_tests dyninstAPI)
  add_test(NAME ${_targ} COMMAND ${_targ} ${DYNINST_REGEX_TEST_FILES})
  set_tests_properties(${_targ} PROPERTIES ENVIRONMENT "${_rt_libs}")

  # These tests use 'codegen.h' directly that needs to have `DYNINST_CODEGEN_ARCH_<ARCH>` defined
  set(_hosts "I386" "X86_64" "PPC64LE" "AARCH64")
  foreach(_h ${_hosts})
    if(${_h})
      target_compile_definitions(${_targ} PRIVATE "DYNINST_CODEGEN_ARCH_${_h}")
      break()
    endif()
  endforeach()
  
  unset(_targ)
  unset(_name)
endmacro()

_mk_test(findObject)
_mk_test(findModule)

unset(_mk_test)
