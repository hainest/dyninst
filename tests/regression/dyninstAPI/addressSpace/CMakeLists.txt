# cmake-format: off

set(_hosts "I386" "X86_64" "PPC64LE" "AARCH64")
set(_tests findObject findModule)

foreach(_t ${_tests})
  foreach(_h ${_hosts})
    if(${_h})
      target_compile_definitions("dyninstapi_addressSpace_${_t}" PRIVATE "DYNINST_CODEGEN_ARCH_${_h}")
      break()
    endif()
  endforeach()

# cmake-format: on
