include_guard(GLOBAL)

include(DyninstCheckStaticExeSupported)
dyninst_check_static_exe_supported(LANGUAGES CXX)

include(DyninstCheckPIESupported)
dyninst_check_pie_supported(LANGUAGES CXX)

set(DYNINST_REGRESSIONTEST_EXECUTABLES "")
set(DYNINST_REGRESSIONTEST_LIBRARIES "")

macro(make_lib_test test_type test_name)
  set(_lib_test_target "DyninstRegTest_${test_type}LIB_${test_name}")
  add_library(${_lib_test_target} ${test_type} regression_test_binaries.cpp)
  list(APPEND DYNINST_REGRESSIONTEST_LIBRARIES ${_lib_test_target})
endmacro()

macro(make_exec_test test_name)
  set(_exec_test_target "DyninstRegTest_EXEC_${test_name}")
  add_executable(${_exec_test_target} regression_test_binaries.cpp)
  target_compile_definitions(${_exec_test_target} PRIVATE "HAS_MAIN")
  list(APPEND DYNINST_REGRESSIONTEST_EXECUTABLES ${_exec_test_target})
endmacro()

# shared libraries
foreach(main_t "hasmain" "none")
  foreach(strip_t "stripped" "none")

    # The '_pic' makes creating the test targets easier
    make_lib_test(SHARED "${main_t}_${strip_t}_pic")

    if(strip_t STREQUAL "stripped")
      target_link_libraries(${_lib_test_target} "-s")
    endif()

    if(main_t STREQUAL "hasmain")
      target_compile_definitions(${_lib_test_target} PRIVATE "HAS_MAIN")
      target_compile_definitions(${_lib_test_target} PRIVATE "ADD_INTERP")
      target_link_options(${_lib_test_target} PRIVATE "-Wl,-e,main")
    endif()

  endforeach()
endforeach()

# Test static libraries
make_lib_test(STATIC "staticlib")

if(DYNINST_CXX_STATIC_EXE_SUPPORTED)
  set(_static_link "staticlink")
endif()

# Test executables
foreach(strip_t "stripped" "none")
  foreach(pie_t "pie" "none")
    foreach(link_t "staticlink" "dynlink")

      if(link_t STREQUAL "staticlink" AND NOT DYNINST_CXX_STATIC_LINK)
        continue()
      endif()

      # static+pic isn't PIE
      if(link_t STREQUAL "staticlink" AND pie_t STREQUAL "pie")
        continue()
      endif()

      make_exec_test("${strip_t}_${pie_t}_${link_t}")

      if(pie_t STREQUAL "pie")
        set_property(TARGET ${_exec_test_target} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
      else()
        # some compilers use PIE by default
        target_compile_options(${_exec_test_target} PRIVATE "-no-pie")
        target_link_options(${_exec_test_target} PRIVATE "-no-pie")
      endif()

      if(strip_t STREQUAL "stripped")
        # '-s' works for gcc and clang
        target_link_libraries(${_exec_test_target} "-s")
      endif()

      if(link_t STREQUAL "staticlink")
        target_link_options(${_exec_test_target} PRIVATE "-static")
      endif()

    endforeach()
  endforeach()
endforeach()

add_library(DyninstRegTest_libraries INTERFACE)
target_link_libraries(DyninstRegTest_libraries
                      INTERFACE "${DYNINST_REGRESSIONTEST_LIBRARIES}")

# Since no test actually links against this target, we can hijack the INTERFACE_LINK_LIBRARIES
# property to export the names of the test executables
add_library(DyninstRegTest_executables INTERFACE)
target_link_libraries(DyninstRegTest_executables
                      INTERFACE "${DYNINST_REGRESSIONTEST_EXECUTABLES}")
