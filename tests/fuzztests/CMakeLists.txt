# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(fuzz_test tcbinfoparser.cpp)
if(USE_LIBSGX)
    target_compile_definitions(fuzz_test PRIVATE OE_USE_LIBSGX )
endif()

add_custom_command( TARGET fuzz_test
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/seeds/tcbinfoparser ${CMAKE_CURRENT_BINARY_DIR}/seeds/tcbinfoparser )

target_link_libraries(fuzz_test oehostapp )


add_test(NAME tests/fuzz_test COMMAND ./fuzz_test ${CMAKE_CURRENT_BINARY_DIR}/seeds/tcbinfoparser)

