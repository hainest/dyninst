include(DyninstRegTests)

# cmake-format: off

dyninst_reg_test(
  dyninstapi_addressSpace_findObject
  findObject.cpp
  dyninstAPI
  NOSTATICLIBS  # BPatch::openBinary can't open archives
)

set(_hosts "DYNINST_HOST_ARCH_I386" "DYNINST_HOST_ARCH_X86_64" "DYNINST_HOST_ARCH_PPC64LE" "DYNINST_HOST_ARCH_AARCH64")

foreach(_h ${_hosts})
  if(${_h})
    target_compile_definitions(dyninstapi_addressSpace_findObject PRIVATE "${_h}")
    break()
  endif()
endforeach()

# cmake-format: on
