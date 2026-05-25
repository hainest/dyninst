include_guard(GLOBAL)

set(_headers function.h instMapping.h instPoint.h patch_edge.h patch.h Patching.h)

set(_sources function.C instPoint.C patch_block.C patch_edge.C Patching.C)

if(DYNINST_OS_UNIX)
  list(APPEND _sources patch_block-unix.C)
elseif(DYNINST_OS_Windows)
  list(APPEND _sources patch_block-winnt.C)
endif()

# cmake-format: off
dyninst_library(
  dyninstAPI_patching
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common parseAPI patchAPI pcontrol
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)
# cmake-format: on

target_include_directories(dyninstAPI_patching PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
