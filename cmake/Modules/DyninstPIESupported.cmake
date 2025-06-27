include_guard(GLOBAL)

# Check if the CMAKE_<LANG>_COMPILER supports position-independent executables
include(CheckPIESupported)
check_pie_supported(LANGUAGES C CXX)

# Use PIE when POSITION_INDEPENDENT_CODE is used for executables
if(POLICY CMP0083)
  cmake_policy(SET CMP0083 NEW)
endif()
