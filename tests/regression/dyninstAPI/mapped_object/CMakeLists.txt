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

set(_hosts "I386" "X86_64" "PPC64LE" "AARCH64")
set(_tests findModule)

foreach(_t ${_tests})
  foreach(_h ${_hosts})
    if(${_h})
      target_compile_definitions("dyninstapi_mapped_object_${_t}" PRIVATE "DYNINST_CODEGEN_ARCH_${_h}")
      break()
    endif()
  endforeach()
endforeach()

# cmake-format: on
