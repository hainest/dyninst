include(DyninstRegTests)

# cmake-format: off

dyninst_reg_test(
  dyninstapi_addressSpace_findObject
  findObject.cpp
  dyninstAPI
  NOSTATICLIBS  # BPatch::openBinary can't open archives
)

<<<<<<< HEAD
dyninst_reg_test(
  dyninstapi_addressSpace_findModule
  findModule.cpp
  dyninstAPI
  NOSTATICLIBS  # BPatch::openBinary can't open archives
  NOSTRIPPEDLIBS
  NOEXEC
)

set(_hosts "I386" "X86_64" "PPC64LE" "AARCH64")
set(_tests findObject findModule)

foreach(_t ${_tests})
  foreach(_h ${_hosts})
    if(${_h})
      target_compile_definitions("dyninstapi_addressSpace_${_t}" PRIVATE "DYNINST_CODEGEN_ARCH_${_h}")
      break()
    endif()
  endforeach()
=======
set(_hosts "DYNINST_HOST_ARCH_I386" "DYNINST_HOST_ARCH_X86_64" "DYNINST_HOST_ARCH_PPC64LE" "DYNINST_HOST_ARCH_AARCH64")

foreach(_h ${_hosts})
  if(${_h})
    target_compile_definitions(dyninstapi_addressSpace_findObject PRIVATE "${_h}")
    break()
  endif()
>>>>>>> bb9c5919c (Update AddressSpace::findObject)
endforeach()

# cmake-format: on
