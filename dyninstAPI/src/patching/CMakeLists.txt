include_guard(GLOBAL)

set(
  _headers
  block.h
  function.h
  instMapping.h
  instPoint.h
  patch_edge.h
  patch.h
  Patching.h
)

set(
  _sources
  block.C
  function.C
  instPoint.C
  patch_edge.C
  Patching.C
)

if(DYNINST_OS_UNIX)
  list(APPEND _sources patch_block-unix.C)
elseif(DYNINST_OS_Windows)
  list(APPEND _sources patch_block-winnt.C)
endif()

dyninst_library(
  dyninstAPI_patching
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_DEPS common parseAPI patchAPI pcontrol
)

set(
  _inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)

target_include_directories(dyninstAPI_patching PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
