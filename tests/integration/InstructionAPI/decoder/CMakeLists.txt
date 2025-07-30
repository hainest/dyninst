include_guard(GLOBAL)

add_library(decode_verify OBJECT decode_verify.cpp decode_verify.h)
target_compile_options(decode_verify PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(decode_verify PRIVATE instructionAPI)
target_include_directories(decode_verify PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(x86)
