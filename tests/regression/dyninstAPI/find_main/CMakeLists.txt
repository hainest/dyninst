include_guard(GLOBAL)

include(DyninstLinkProperties)
include(Dyninst32bitRuntimeFlag)
include(DyninstPIESupported)

add_executable(find_main find_main.cpp)
target_compile_options(find_main PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(find_main PRIVATE symtabAPI dyninstAPI)

foreach(lang_t "C" "CXX")
  foreach(pie_t "pie" "nopie")
    foreach(link_t "stat" "dyn")
      foreach(bits_t "32" "64")
        if(pie_t STREQUAL "pie" AND link_t STREQUAL "dyn" AND bits_t STREQUAL "32")
          continue()
        endif()
        foreach(strip_t "stripped" "nostripped")

          set(_targ "${lang_t}_${link_t}_${pie_t}_${bits_t}_${strip_t}")
          add_executable(${_targ} find_main_test.c)
  
          set_property(TARGET ${_targ} PROPERTY LANGUAGE ${_lang_t})
  
          set(_pie FALSE)
          if(pie_t STREQUAL "pie")
            set(_pie TRUE)
          endif()
          set_property(TARGET ${_targ} PROPERTY POSITION_INDEPENDENT_CODE ${_pie})

          unset(_slt)
          if(link_t STREQUAL "stat")
            set(_slt "STATIC_")
          endif()

          if(bits_t STREQUAL "32")
            if(DYNINST_${lang_t}_HAVE_32BIT_${_slt}RUNTIME_FLAG)
              target_compile_options(${_targ} PRIVATE ${DYNINST_${lang_t}_32BIT_${_slt}RUNTIME_FLAG})
              target_link_libraries(${_targ} PRIVATE ${DYNINST_${lang_t}_32BIT_${_slt}RUNTIME_FLAG})
            endif()
          else()            
            if(DYNINST_${lang_t}_HAVE_${_slt}LINK_FLAG)
              target_link_libraries(${_targ} PRIVATE ${DYNINST_${lang_t}_${_slt}LINK_FLAG})
            endif()
          endif()
  
          if(strip_t STREQUAL "stripped")
            # '-s' works for gcc and clang
            target_link_libraries(${_targ} PRIVATE "-s")
          endif()
        endforeach()

        set(_input_name "${lang_t}_${link_t}_${pie_t}_${bits_t}")
        set(_input_stripped "$<TARGET_FILE_NAME:${_input_name}_stripped>")
        set(_input_nostripped "$<TARGET_FILE_NAME:${_input_name}_nostripped>")
        set(_test_name "dyninstAPI_find_main_${_input_name}")
        add_test(NAME ${_test_name} COMMAND find_main ${_input_nostripped}
                                            ${_input_stripped})
        set_tests_properties(${_test_name} PROPERTIES LABELS "regression")

      endforeach()
    endforeach()
  endforeach()
endforeach()
