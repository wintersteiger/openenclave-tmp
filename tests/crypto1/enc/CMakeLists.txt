# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

include(add_enclave_executable)
add_enclave_executable(crypto1_enc sign.conf private.pem
    enc.cpp
    )

target_compile_options(crypto1_enc PRIVATE
    -std=c++11
)

target_link_libraries(crypto1_enc oelibcxx oeenclave)
