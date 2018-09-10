// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/host.h>
#include <openenclave/internal/tests.h>
#include "../args.h"

#if defined(__linux__)
#include <sys/sysinfo.h>
#endif

#define SKIP_RETURN_CODE 2

/* Get the system memory size (RAM plus swap) */
uint64_t get_free_system_memory(void)
{
#if defined(__linux__)
    struct sysinfo info;

    if (sysinfo(&info) != 0)
        return 0;

    return info.freeram + info.freeswap;
#else
    return 0L;
#endif
}

int main(int argc, const char* argv[])
{
#if !defined(OE_USE_LIBSGX)

    oe_result_t result;
    const oe_enclave_type_t type = OE_ENCLAVE_TYPE_SGX;
    const uint32_t flags = oe_get_create_flags();
    oe_enclave_t* enclave = NULL;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    /* This system must have at least 64 gigabytes of free system memory */
    {
        const uint64_t GIGABYTE = 0x0000000040000000;
        const uint64_t REQUIRED_MEMORY = 64 * GIGABYTE;
        uint64_t free_memory = get_free_system_memory();

        if (free_memory < REQUIRED_MEMORY)
        {
            fprintf(
                stderr,
                "%s: warning: insufficient memory to load enclave: %lu\n",
                argv[0],
                free_memory);

            /* Skip test on machines with insufficient memory */
            return SKIP_RETURN_CODE;
        }
    }

    result = oe_create_enclave(argv[1], type, flags, NULL, 0, &enclave);
    OE_TEST(result == OE_OK);

    args_t args;
    args.result = OE_UNEXPECTED;
    result = oe_call_enclave(enclave, "test_malloc", &args);
    OE_TEST(result == OE_OK);
    OE_TEST(args.result == OE_OK);

    oe_terminate_enclave(enclave);

    printf("=== passed all tests (%s)\n", argv[0]);

#endif /* defined(OE_USE_LIBSGX) */

    return 0;
}
