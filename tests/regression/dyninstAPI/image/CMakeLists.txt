include(DyninstRegTests)

# cmake-format: off

dyninst_reg_test(
  dyninstapi_image_findModule
  findModule.cpp
  dyninstAPI
  NOSTRIPPEDLIBS  # Needs debuginfo
  NOSTRIPPEDEXEC
  NOSTATICLIBS
)

# cmake-format: on
