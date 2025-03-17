include(DyninstRegTests)

# cmake-format: off

dyninst_reg_test(
  dyninstapi_addressSpace_findObject
  findObject.cpp
  dyninstAPI
  NOSTATICLIBS  # BPatch::openBinary can't open archives
)

dyninst_reg_test(
  dyninstapi_addressSpace_findModule
  findModule.cpp
  dyninstAPI
  NOSTATICLIBS  # BPatch::openBinary can't open archives
  NOSTRIPPEDLIBS
  NOEXEC
)

set(_hosts "DYNINST_HOST_ARCH_I386" "DYNINST_HOST_ARCH_X86_64" "DYNINST_HOST_ARCH_PPC64LE" "DYNINST_HOST_ARCH_AARCH64")
set(_tests findObject findModule)

foreach(_t ${_tests})
  foreach(_h ${_hosts})
    if(${_h})
      target_compile_definitions("dyninstapi_addressSpace_${_t}" PRIVATE "${_h}")
      break()
    endif()
  endforeach()
endforeach()

# cmake-format: on
