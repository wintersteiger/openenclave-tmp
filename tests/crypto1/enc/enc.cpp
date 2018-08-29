// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#include <openenclave/enclave.h>
#include "../common/args.h"
#include "../common/tests.cpp"

OE_ECALL void ecall_test_cert_chain(void* args_)
{
    test_cert_chain_args_t* args = (test_cert_chain_args_t*)args_;
    test_cert_chain(args->root, args->intermediate, args->leaf);
}
