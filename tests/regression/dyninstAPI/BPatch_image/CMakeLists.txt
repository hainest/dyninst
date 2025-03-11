include(DyninstRegTests)

# cmake-format: off

dyninst_reg_test(
  dyninstapi_BPatch_image_findFunction
  findFunction.cpp
  dyninstAPI
  NOSTATICLIBS
  NOSTRIPPEDEXEC
)

dyninst_reg_test(
  dyninstapi_BPatch_image_findModule
  findModule.cpp
  dyninstAPI
  NOSTRIPPED  # Needs debuginfo
)

# cmake-format: on
