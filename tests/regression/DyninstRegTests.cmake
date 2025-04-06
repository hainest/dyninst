set(_bin_proj_dir "${PROJECT_SOURCE_DIR}/tests/regression/binaries/")

function(dyninst_reg_test _testdriver _source_files _dynlibs)
  # cmake-format: off
  set(_keywords
    NOSTATICLIBS    # exclude static libraries 
    NOSTRIPPEDLIBS  # exclude stripped binaries
    NOEXEC          # exclude executables
    NOSTRIPPEDEXEC  # exclude stripped executables
  )
  # cmake-format: on
  
  cmake_parse_arguments(PARSE_ARGV 0 _reg_test "${_keywords}" "" "")
  
  set(_test_types libraries)
  if(NOT _reg_test_NOEXEC)
    list(APPEND _test_types executables)
  endif()
  
  set(_no_stripped FALSE)
  if(_reg_test_NOSTRIPPEDLIBS OR _reg_test_NOSTRIPPEDEXEC)
    set(_no_stripped TRUE)
  endif()
  
  set(_binaries_to_test "")

  foreach(_type ${_test_types})

    # Get all of the binaries built in the 'binaries' project
    get_property(
      _all_test_binaries
      TARGET DyninstRegTest_${_type}
      PROPERTY INTERFACE_LINK_LIBRARIES)
    
    foreach(_bin ${_all_test_binaries})
      if(_reg_test_NOSTATICLIBS AND _bin MATCHES "STATICLIB")
        continue()
      endif()
      
      if(_no_stripped AND _bin MATCHES "stripped")
        continue()
      endif()
      
      list(APPEND _binaries_to_test "$<TARGET_FILE:${_bin}>")
    
    endforeach()

  endforeach()
  
  add_executable(${_testdriver} ${_source_files})
  target_compile_options(${_testdriver} PRIVATE ${UNIT_TESTS_WARNING_FLAGS})
  target_include_directories(${_testdriver} PRIVATE "${_bin_proj_dir}")
  target_link_libraries(${_testdriver} PRIVATE ${_dynlibs})
  
  add_test(NAME ${_testdriver} COMMAND ${_testdriver} ${_binaries_to_test}
                                       COMMAND_EXPAND_LISTS)

  set_tests_properties(${_testdriver} PROPERTIES LABELS "regression")

endfunction()
