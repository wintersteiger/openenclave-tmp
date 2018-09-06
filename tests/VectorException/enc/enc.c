// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/atexit.h>
#include <openenclave/internal/calls.h>
#include <openenclave/internal/enclavelibc.h>
#include <openenclave/internal/fault.h>
#include <openenclave/internal/globals.h>
#include <openenclave/internal/jump.h>
#include <openenclave/internal/print.h>
#include <openenclave/internal/reloc.h>
#include <openenclave/internal/sgxtypes.h>
#include <openenclave/internal/trace.h>
#include "../args.h"

// This function will generate the divide by zero function.
// The handler will catch this exception and fix it, and continue execute.
// It will return 0 if success.
int DivideByZeroExceptionFunction(void)
{
    // Making ret, f and d volatile to prevent optimization
    volatile int ret = 1;
    volatile float f = 0;
    volatile double d = 0;

    f = 0.31;
    d = 0.32;

    // Using inline assembly for idiv to prevent it being optimized out
    // completely
    asm volatile(
        "idiv %3"
        : "=a"(ret)
        : "a"(0), "d"(0), "r"(0) // Divisor of 0 is hard-coded
        : "%2", "cc"); // cc indicates that flags will be clobbered by ASM

    // Check if the float registers are recovered correctly after the exception
    // is handled.
    // Please note that this register integrity testing is prone to be skipped
    // with a different compiler/build. This will require that this entire
    // function be written in assembly.
    if (f < 0.309 || f > 0.321 || d < 0.319 || d > 0.321)
    {
        return -1;
    }

    return 0;
}

uint64_t TestDivideByZeroHandler(oe_exception_record_t* exception_record)
{
    if (exception_record->code != OE_EXCEPTION_DIVIDE_BY_ZERO)
    {
        return OE_EXCEPTION_CONTINUE_SEARCH;
    }

    // Skip the idiv instruction - 2 is tied to the size of the idiv instruction
    // and can change with a different compiler/build. Minimizing this with the
    // use of the inline assembly for integer division
    exception_record->context->rip += 2;
    return OE_EXCEPTION_CONTINUE_EXECUTION;
}

#define MAX_EXCEPTION_HANDLER_COUNT 64

#define PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_) \
    uint64_t __exception_handler_name_(                          \
        oe_exception_record_t* exception_record)                 \
    {                                                            \
        return OE_EXCEPTION_CONTINUE_SEARCH;                     \
    }

#define TEN_PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_) \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_0)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_1)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_2)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_3)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_4)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_5)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_6)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_7)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_8)     \
    PASSTHROUGH_EXCEPTION_HANDLER(__exception_handler_name_prefix_##_9)

// Define 64 pass through exception handlers.
TEN_PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler0)
TEN_PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler1)
TEN_PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler2)
TEN_PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler3)
TEN_PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler4)
TEN_PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler5)
PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler6_0)
PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler6_2)
PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler6_1)
PASSTHROUGH_EXCEPTION_HANDLER(TestPassThroughHandler6_3)

#define TEN_EXCEPTION_HANDLER_POINTERS(__exception_handler_name_prefix_) \
    __exception_handler_name_prefix_##_0,                                \
        __exception_handler_name_prefix_##_1,                            \
        __exception_handler_name_prefix_##_2,                            \
        __exception_handler_name_prefix_##_3,                            \
        __exception_handler_name_prefix_##_4,                            \
        __exception_handler_name_prefix_##_5,                            \
        __exception_handler_name_prefix_##_6,                            \
        __exception_handler_name_prefix_##_7,                            \
        __exception_handler_name_prefix_##_8,                            \
        __exception_handler_name_prefix_##_9,

static oe_vectored_exception_handler_t
    g_test_pass_through_handlers[MAX_EXCEPTION_HANDLER_COUNT] = {
        TEN_EXCEPTION_HANDLER_POINTERS(TestPassThroughHandler0)
            TEN_EXCEPTION_HANDLER_POINTERS(TestPassThroughHandler1)
                TEN_EXCEPTION_HANDLER_POINTERS(TestPassThroughHandler2)
                    TEN_EXCEPTION_HANDLER_POINTERS(TestPassThroughHandler3)
                        TEN_EXCEPTION_HANDLER_POINTERS(TestPassThroughHandler4)
                            TEN_EXCEPTION_HANDLER_POINTERS(
                                TestPassThroughHandler5)
                                TestPassThroughHandler6_0,
        TestPassThroughHandler6_1,
        TestPassThroughHandler6_2,
        TestPassThroughHandler6_3};

static oe_vectored_exception_handler_t g_test_div_by_zero_handler;

int VectorExceptionSetup()
{
    uint64_t ret = -1;
    oe_result_t result;

    // Add one exception handler.
    result = oe_add_vectored_exception_handler(false, TestDivideByZeroHandler);
    if (result != OE_OK)
    {
        return ret;
    }

    // Remove the exception handler.
    if (oe_remove_vectored_exception_handler(TestDivideByZeroHandler) != OE_OK)
    {
        return -1;
    }

    // Insert the exception handler to the front.
    result = oe_add_vectored_exception_handler(true, TestDivideByZeroHandler);
    if (result != OE_OK)
    {
        return ret;
    }

    // Remove the exception handler.
    if (oe_remove_vectored_exception_handler(TestDivideByZeroHandler) != OE_OK)
    {
        return -1;
    }

    // Append one by one till reach the max.
    for (uint32_t i = 0; i < OE_COUNTOF(g_test_pass_through_handlers); i++)
    {
        result = oe_add_vectored_exception_handler(
            false, g_test_pass_through_handlers[i]);
        if (result != OE_OK)
        {
            return ret;
        }
    }

    // Can't add one more.
    result = oe_add_vectored_exception_handler(false, TestDivideByZeroHandler);
    if (result == OE_OK)
    {
        return ret;
    }

    // Remove all registered handlers.
    for (uint32_t i = 0; i < OE_COUNTOF(g_test_pass_through_handlers); i++)
    {
        if (oe_remove_vectored_exception_handler(
                g_test_pass_through_handlers[i]) != OE_OK)
        {
            return -1;
        }
    }

    // Add handles to the front one by one till reach the max.
    for (uint32_t i = 0; i < OE_COUNTOF(g_test_pass_through_handlers); i++)
    {
        result = oe_add_vectored_exception_handler(
            true, g_test_pass_through_handlers[i]);
        if (result != OE_OK)
        {
            return ret;
        }
    }

    // Can't add one more.
    result = oe_add_vectored_exception_handler(true, TestDivideByZeroHandler);
    if (result == OE_OK)
    {
        return ret;
    }

    // Remove all registered handlers.
    for (uint32_t i = 0; i < OE_COUNTOF(g_test_pass_through_handlers); i++)
    {
        if (oe_remove_vectored_exception_handler(
                g_test_pass_through_handlers[i]) != OE_OK)
        {
            return -1;
        }
    }

    // Add the test pass through handlers.
    for (uint32_t i = 0; i < OE_COUNTOF(g_test_pass_through_handlers) - 1; i++)
    {
        result = oe_add_vectored_exception_handler(
            false, g_test_pass_through_handlers[i]);
        if (result != OE_OK)
        {
            return ret;
        }
    }

    // Add the real handler to the end.
    g_test_div_by_zero_handler = TestDivideByZeroHandler;
    result = oe_add_vectored_exception_handler(false, TestDivideByZeroHandler);
    if (result != OE_OK)
    {
        return ret;
    }

    ret = 0;
    return ret;
}

int VectorExceptionCleanup()
{
    // Remove all handlers.
    if (oe_remove_vectored_exception_handler(g_test_div_by_zero_handler) !=
        OE_OK)
    {
        return -1;
    }

    for (uint32_t i = 0; i < OE_COUNTOF(g_test_pass_through_handlers) - 1; i++)
    {
        if (oe_remove_vectored_exception_handler(
                g_test_pass_through_handlers[i]) != OE_OK)
        {
            return -1;
        }
    }

    return 0;
}

OE_ECALL void TestVectorException(void* args_)
{
    TestVectorExceptionArgs* args = (TestVectorExceptionArgs*)args_;
    args->ret = -1;

    if (!oe_is_outside_enclave(args, sizeof(TestVectorExceptionArgs)))
    {
        return;
    }

    if (VectorExceptionSetup() != 0)
    {
        return;
    }

    oe_host_printf(
        "TestVectorException: will generate a hardware exception "
        "inside enclave!\n");
    if (DivideByZeroExceptionFunction() != 0)
    {
        args->ret = -1;
        return;
    }

    oe_host_printf(
        "TestVectorException: hardware exception is handled correctly!\n");

    if (VectorExceptionCleanup() != 0)
    {
        return;
    }

    args->ret = 0;
    return;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* AllowDebug */
    1024, /* HeapPageCount */
    1024, /* StackPageCount */
    2);   /* TCSCount */
