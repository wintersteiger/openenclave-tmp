// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/internal/cert.h>
#include <openenclave/internal/tests.h>
#include <string>
#include <stdio.h>

oe_result_t create_chain(
    const char* c1,
    const char* c2,
    const char* c3,
    oe_cert_chain_t* chain
)
{
    std::string s = "";
    if (c1)
        s += c1;
    if (c2)
        s += c2;
    if (c3)
        s += c3;

    oe_cert_chain_free(chain);
    // size + 1 to include null character.
    oe_result_t result =  oe_cert_chain_read_pem(chain, &s[0], s.size()+1);
    if (result != OE_OK) {
        printf("create_chain result = %s\n", oe_result_str(result));
        fflush(stdout);
    }
    return result;
}

void test_cert_chain(
    const char* root,
    const char* intermediate,
    const char* leaf
)
{
    oe_cert_chain_t chain = {0};

    // Create root, intermediate, leaf chain.
    // The following passes in host, but fails in enclave.
    OE_TEST(create_chain(root, intermediate, leaf, &chain) == OE_OK);    

    // The following should fail since the order of certs is different. 
    // But passes in host.
    OE_TEST(create_chain(leaf, intermediate, root, &chain) == OE_OK);
    OE_TEST(create_chain(intermediate, root, leaf, &chain) == OE_OK);

    oe_cert_chain_free(&chain);
}