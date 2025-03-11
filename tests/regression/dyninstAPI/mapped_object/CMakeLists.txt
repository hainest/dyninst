include(DyninstRegTests)

# cmake-format: off

dyninst_reg_test(
  dyninstapi_mapped_object_findModule
  findModule.cpp
  dyninstAPI
  NOSTATICLIBS
  NOSTRIPPEDLIBS  # Needs debuginfo
  NOEXEC
)

set(_hosts "DYNINST_HOST_ARCH_I386" "DYNINST_HOST_ARCH_X86_64" "DYNINST_HOST_ARCH_PPC64LE" "DYNINST_HOST_ARCH_AARCH64")
set(_tests findModule)

foreach(_t ${_tests})
  foreach(_h ${_hosts})
    if(${_h})
      target_compile_definitions("dyninstapi_mapped_object_${_t}" PRIVATE "${_h}")
      break()
    endif()
  endforeach()
endforeach()

# cmake-format: on
