// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_CALLS_H
#define _OE_CALLS_H

#include <openenclave/bits/defs.h>
#include <openenclave/bits/types.h>
#include <openenclave/internal/cpuid.h>
#include <openenclave/internal/defs.h>
#include "backtrace.h"
#include "sgxtypes.h"

OE_EXTERNC_BEGIN

typedef struct _oe_enclave oe_enclave_t;

typedef void (*oe_ecall_function)(uint64_t argIn, uint64_t* argOut);

typedef void (*oe_ocall_function)(uint64_t argIn, uint64_t* argOut);

/*
**==============================================================================
**
** The flags parameter for oe_ocall()
**
** Flags stack with the ones on the current thread (i.e., are or'd together)
** for the duration of the ocall.
**
**==============================================================================
*/

/*
**==============================================================================
**
** oe_code_t
**
**     The code parameter for oe_ecall() and oe_ocall()
**
**==============================================================================
*/

typedef enum _oe_code {
    OE_CODE_NONE = 0,
    OE_CODE_ECALL = 1,
    OE_CODE_ERET = 2,
    OE_CODE_OCALL = 3,
    OE_CODE_ORET = 4,
    __OE_CODE_MAX = OE_ENUM_MAX,
} oe_code_t;

OE_STATIC_ASSERT(sizeof(oe_code_t) == sizeof(unsigned int));

/*
**==============================================================================
**
** oe_func_t
**
**     The func parameter for oe_ecall() and oe_ocall()
**
**==============================================================================
*/

/* ECALL function numbers are in the range: [0:32765] */
#define OE_ECALL_BASE 0

/* OCALL function numbers are in the range: [32768:65535] */
#define OE_OCALL_BASE 0x8000

/* Function numbers are 16 bit integers */
typedef enum _oe_func {
    OE_ECALL_DESTRUCTOR = OE_ECALL_BASE,
    OE_ECALL_INIT_ENCLAVE,
    OE_ECALL_CALL_ENCLAVE,
    OE_ECALL_VERIFY_REPORT,
    OE_ECALL_GET_SGX_REPORT,
    OE_ECALL_VIRTUAL_EXCEPTION_HANDLER,
    /* Caution: always add new ECALL function numbers here */

    OE_OCALL_CALL_HOST = OE_OCALL_BASE,
    OE_OCALL_CALL_HOST_BY_ADDRESS,
    OE_OCALL_GET_QE_TARGET_INFO,
    OE_OCALL_GET_QUOTE,
    OE_OCALL_GET_REVOCATION_INFO,
    OE_OCALL_THREAD_WAKE,
    OE_OCALL_THREAD_WAIT,
    OE_OCALL_THREAD_WAKE_WAIT,
    OE_OCALL_MALLOC,
    OE_OCALL_REALLOC,
    OE_OCALL_FREE,
    OE_OCALL_PUTS,
    OE_OCALL_PUTCHAR,
    OE_OCALL_PRINT,
    OE_OCALL_SLEEP,
    OE_OCALL_GET_TIME,
    OE_OCALL_BACKTRACE_SYMBOLS,
    /* Caution: always add new OCALL function numbers here */

    __OE_FUNC_MAX = OE_ENUM_MAX,
} oe_func_t;

OE_STATIC_ASSERT(sizeof(oe_func_t) == sizeof(unsigned int));

#define OE_EXCEPTION_CONTINUE_SEARCH 0x0
#define OE_EXCEPTION_CONTINUE_EXECUTION 0xFFFFFFFF

/*
**==============================================================================
**
** oe_make_call_arg1()
**
**     Form the 'arg1' parameter to both oe_enter() and oe_exit(). This
**     parameter is a 64-bit integer that contains:
**
**         code -- indicating whether ECALL, OCALL, ERET, or ORET
**         func -- the number of the function being called
**         flags -- any bit flags
**         result -- the result of the transport (not the function)
**
**==============================================================================
*/

OE_INLINE uint64_t oe_make_call_arg1(
    oe_code_t code,
    oe_func_t func,
    uint16_t flags,
    oe_result_t result)
{
    /* [ CODE:16 | FUNC:16 | FLAGS:16 | RESULT:16 ] */
    return ((uint64_t)code << 48) | ((uint64_t)func << 32) |
           ((uint64_t)flags << 16) | ((uint64_t)result);
}

/*
**==============================================================================
**
** oe_get_code_from_call_arg1()
**
**==============================================================================
*/

OE_INLINE oe_code_t oe_get_code_from_call_arg1(uint64_t arg)
{
    return (oe_code_t)((0xffff000000000000 & arg) >> 48);
}

/*
**==============================================================================
**
** oe_get_func_from_call_arg1()
**
**==============================================================================
*/

OE_INLINE uint16_t oe_get_func_from_call_arg1(uint64_t arg)
{
    return (oe_func_t)((0x0000ffff00000000 & arg) >> 32);
}

/*
**==============================================================================
**
** oe_get_flags_from_call_arg1()
**
**==============================================================================
*/

OE_INLINE uint16_t oe_get_flags_from_call_arg1(uint64_t arg)
{
    return (uint16_t)((0x00000000ffff0000 & arg) >> 16);
}

/*
**==============================================================================
**
** oe_get_result_from_call_arg1()
**
**==============================================================================
*/

OE_INLINE uint16_t oe_get_result_from_call_arg1(uint64_t arg)
{
    return (uint16_t)(0x000000000000ffff & arg);
}

/*
**==============================================================================
**
** oe_call_enclave_args_t
**
**==============================================================================
*/

typedef void (*oe_enclave_func_t)(void* args);

typedef struct _oe_call_enclave_args
{
    uint64_t func;
    uint64_t vaddr;
    void* args;
    oe_result_t result;
} oe_call_enclave_args_t;

/*
**==============================================================================
**
** oe_host_func_t:
**
**     The oe_call_host() enclave function is dispatched to a host function
**     with the following prototype.
**
**         OE_OCALL void (*)(void* args, oe_enclave_t* enclave);
**
**     Host-application developers may legally omit one or more parameters, and
**     therefore define functions with the following prototypes (where the last
**     is the maximal form).
**
**         OE_OCALL void (*)(void);
**         OE_OCALL void (*)(void* args);
**         OE_OCALL void (*)(void* args, oe_enclave_t* enclave);
**
**     This pattern is common in the C language where the following forms of
**     main() are prevalent.
**
**         int main(void);
**         int main(int argc, char* argv[]);
**         int main(int argc, char* argv[], char* envp[]);
**
**     In this case the _start() function passes all parameters but the main()
**     prototype may omit parameters from right to left.
**
**==============================================================================
*/

typedef void (*oe_host_func_t)(void* args, oe_enclave_t* enclave);

/*
**==============================================================================
**
** oe_call_host_args_t
**
**==============================================================================
*/

typedef struct _oe_call_host_args
{
    void* args;
    oe_result_t result;
    OE_ZERO_SIZED_ARRAY char func[];
} oe_call_host_args_t;

/*
**==============================================================================
**
** oe_call_host_by_address_args_t
**
**==============================================================================
*/

typedef struct _oe_call_host_by_address_args
{
    void* args;
    oe_host_func_t func;
    oe_result_t result;
} oe_call_host_by_address_args_t;

/*
**==============================================================================
**
** oe_print_args_t
**
**     Print 'str' to stdout (device == 0) or stderr (device == 1).
**
**==============================================================================
*/

typedef struct _oe_print_args
{
    int device;
    char* str;
} oe_print_args_t;

/*
**==============================================================================
**
** oe_realloc_args_t
**
**     void* realloc(void* ptr, size_t size)
**
**==============================================================================
*/

typedef struct _oe_realloc_args
{
    void* ptr;
    size_t size;
} oe_realloc_args_t;

/*
**==============================================================================
**
** oe_init_enclave_args_t
**
**     Runtime state to initialize enclave state with, includes
**     - First 8 leaves of CPUID for enclave emulation
**     - Enclave handle obtained by oe_create_enclave()
**
**==============================================================================
*/

typedef struct _oe_init_enclave_args
{
    uint32_t cpuidTable[OE_CPUID_LEAF_COUNT][OE_CPUID_REG_COUNT];
    oe_enclave_t* enclave;
} oe_init_enclave_args_t;

/*
**==============================================================================
**
** oe_backtrace_symbols_args_t
**
**     Ask host to print a backtrace collected by the enclave using the
**     oe_backtrace() function.
**
**==============================================================================
*/

typedef struct _oe_backtrace_symbols_args
{
    void* buffer[OE_BACKTRACE_MAX];
    int size;
    char** ret;
} oe_backtrace_symbols_args_t;

/**
 * Perform a low-level enclave function call (ECALL).
 *
 * This function performs a low-level enclave function call by invoking the
 * function indicated by the **func** parameter. The enclave defines a
 * corresponding function with the following signature.
 *
 *     void (*)(uint64_t argIn, uint64_t* argOut);
 *
 * The meaning of the **argIn** arg **argOut** parameters is defined by the
 * implementer of the function and either may be null.
 *
 * Open Enclave uses the low-level ECALL interface to implement internal calls,
 * used by oe_call_enclave() and oe_terminate_enclave(). Enclave application
 * developers are encouraged to use oe_call_enclave() instead.
 *
 * At the software layer, this function sends an **ECALL** message to the
 * enclave and waits for an **ERET** message. Note that the ECALL implementation
 * may call back into the host (an OCALL) before returning.
 *
 * At the hardware layer, this function executes the **ENCLU.EENTER**
 * instruction to enter the enclave. When the enclave returns from the ECALL,
 * it executes the **ENCLU.EEXIT** instruction exit the enclave and to resume
 * host execution.
 *
 * Note that the return value only indicates whether the ECALL was called and
 * not whether it was successful. The ECALL implementation must define its own
 * error reporting scheme based on its parameters.
 *
 * @param func The number of the function to be called.
 * @param argsIn The input argument passed to the function.
 * @param argOut The output argument passed back from the function.
 *
 * @retval OE_OK The function was successful.
 * @retval OE_FAILED The function failed.
 * @retval OE_INVALID_PARAMETER One or more parameters is invalid.
 * @retval OE_OUT_OF_THREADS No enclave threads are available to make the call.
 * @retval OE_UNEXPECTED An unexpected error occurred.
 *
 */
oe_result_t oe_ecall(
    oe_enclave_t* enclave,
    uint16_t func,
    uint64_t argIn,
    uint64_t* argOut);

/**
 * Perform a low-level host function call (OCALL).
 *
 * This function performs a low-level host function call by invoking the
 * function indicated by the **func** parameter. The host defines a
 * corresponding function with the following signature.
 *
 *     void (*)(uint64_t argIn, uint64_t* argOut);
 *
 * The meaning of the **argIn** arg **argOut** parameters is defined by the
 * implementer of the function and either may be null.
 *
 * Open Enclave uses this interface to implement internal calls. Enclave
 * application developers are encouraged to use oe_call_host() instead.
 *
 * At the software layer, this function sends an **OCALL** message to the
 * enclave and waits for an **ORET** message. Note that the OCALL implementation
 * may call back into the enclave (an ECALL) before returning.
 *
 * At the hardware layer, this function executes the **ENCLU.EEXIT**
 * instruction to exit the enclave. When the host returns from the OCALL,
 * it executes the **ENCLU.EENTER** instruction to reenter the enclave and
 * resume execution.
 *
 * Note that the return value only indicates whether the OCALL was called
 * not whether it was successful. The ECALL implementation must define its own
 * error reporting scheme based on its parameters.
 *
 * @param func The number of the function to be called.
 * @param argIn The input argument passed to the function.
 * @param argOut The output argument passed back from the function.
 *
 * @retval OE_OK The function was successful.
 * @retval OE_FAILED The function failed.
 * @retval OE_INVALID_PARAMETER One or more parameters is invalid.
 * @retval OE_OUT_OF_THREADS No enclave threads are available to make the call.
 * @retval OE_UNEXPECTED An unexpected error occurred.
 *
 */
oe_result_t oe_ocall(uint16_t func, uint64_t argIn, uint64_t* argOut);

OE_EXTERNC_END

#endif /* _OE_CALLS_H */
