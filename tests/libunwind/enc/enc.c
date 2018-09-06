// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <errno.h>
#include <openenclave/enclave.h>
#include <openenclave/internal/calls.h>
#include <openenclave/internal/enclavelibc.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../host/args.h"
#include "pid.h"

uint32_t g_pid;

int main(int argc, const char* argv[]);

void _exit(int status)
{
    oe_call_host("ExitOCall", (void*)(uint64_t)status);
    abort();
}

void _Exit(int status)
{
    _exit(status);
    abort();
}

void exit(int status)
{
    _exit(status);
    abort();
}

int t_status = 0;

int t_printf(const char* s, ...)
{
    va_list ap;
    char buf[512];

    t_status = 1;
    va_start(ap, s);
    int n = vsnprintf(buf, sizeof buf, s, ap);
    va_end(ap);

    printf("%s\n", buf);
    return n;
}

int t_setrlim(int r, int64_t lim)
{
    return 0;
}

extern char** __environ;

extern const char* __test__;
OE_ECALL void Test(Args* args)
{
    g_pid = args->pid;
    if (args)
    {
        printf("RUNNING: %s\n", __TEST__);
        if (!(__environ = (char**)calloc(1, sizeof(char**))))
            args->ret = 1;

        static const char* argv[] = {
            "test", NULL,
        };
        args->ret = main(1, argv);
        args->test = oe_host_strndup(__TEST__, OE_SIZE_MAX);

        free(__environ);
    }
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* AllowDebug */
    1024, /* HeapPageCount */
    1024, /* StackPageCount */
    2);   /* TCSCount */
