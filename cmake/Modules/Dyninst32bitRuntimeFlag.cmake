include_guard(GLOBAL)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(DyninstLinkProperties)

# Check if the CMAKE_<LANG>_COMPILER supports a 32-bit runtime

# gcc and clang both use 'm32', but allow for other compilers
set(_possible_32bit_flags "-m32")

foreach(_flag ${_possible_32bit_flags})

  set(_saved_link_opts ${CMAKE_REQUIRED_LINK_OPTIONS})

  set(CMAKE_REQUIRED_LINK_OPTIONS "${_flag}")

  foreach(_lang C CXX)
    set(_hslf DYNINST_${_lang}_HAVE_STATIC_LINK_FLAG)
    set(_slf DYNINST_${_lang}_STATIC_LINK_FLAG)
    set(_h32rf DYNINST_${_lang}_HAVE_32BIT_RUNTIME_FLAG)
    set(_32rf DYNINST_${_lang}_32BIT_RUNTIME_FLAG)
    set(_h32srf DYNINST_${_lang}_HAVE_32BIT_STATIC_RUNTIME_FLAG)
    set(_32srf DYNINST_${_lang}_32BIT_STATIC_RUNTIME_FLAG)

    if(_lang STREQUAL C)
      check_c_compiler_flag("${_flag}" ${_h32rf})
      if(${_hslf})
        check_c_compiler_flag("${${_slf}} ${_flag}" ${_h32srf})
      endif()
    else()
      check_cxx_compiler_flag("${_flag}" ${_h32rf})
      if(${_hslf})
        check_cxx_compiler_flag("${${_slf}} ${_flag}" ${_h32srf})
      endif()
    endif()

    if(${_h32rf} AND NOT ${_32rf})
      set(${_32rf}
          "${_flag}"
          CACHE STRING "${_lang} 32-bit runtime flag" FORCE)
    endif()

    if(${_h32srf} AND NOT ${_32srf})
      set(${_32srf}
          "${${_slf}};${_flag}"
          CACHE STRING "${_lang} static 32-bit runtime flag" FORCE)
    endif()
  endforeach()

  set(CMAKE_REQUIRED_LINK_OPTIONS "${_saved_link_opts}")

endforeach()
