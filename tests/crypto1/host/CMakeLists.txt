# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(crypto1_host host.cpp)

add_custom_command(TARGET crypto1_host
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../data ${CMAKE_CURRENT_BINARY_DIR}/../data
)

target_compile_options(crypto1_host PRIVATE
    -std=c++11
)


target_link_libraries(crypto1_host oehostapp)
