// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/**
 * @file result.h
 *
 * This file defines Open Enclave return codes (results).
 *
 */
#ifndef _OE_BITS_RESULT_H
#define _OE_BITS_RESULT_H

#include "defs.h"
#include "types.h"

OE_EXTERNC_BEGIN

/**
 * This enumeration type defines return codes for Open Enclave functions.
 * These functions return **OE_OK** upon success and one of the other
 * enumeration values on failure.
 */
typedef enum _oe_result {

    /**
     * The function was successful.
     */
    OE_OK,

    /**
     * The function failed (without a more specific error code).
     */
    OE_FAILURE,

    /**
     * One or more output buffer function parameters is too small.
     */
    OE_BUFFER_TOO_SMALL,

    /**
     * One or more input function parameters is invalid. Either the value of
     * an input parameter is invalid or a required input parameter pointer
     * is null.
     */
    OE_INVALID_PARAMETER,

    /**
     * The host attempted to perform a reentrant **ECALL**. This occurs when an
     * **OCALL** function attempts to perform an **ECALL**.
     */
    OE_REENTRANT_ECALL,

    /**
     * The function is out of memory. This usually occurs when **malloc** or
     * a related function returns null.
     */
    OE_OUT_OF_MEMORY,

    /**
     * The function is unable to bind the current host thread to an enclave
     * thread. This occurs when the host performs an **ECALL** while all enclave
     * threads are in use.
     */
    OE_OUT_OF_THREADS,

    /**
     * The function encountered an unexpected failure.
     */
    OE_UNEXPECTED,

    /**
     * A cryptographic verification failed. Examples include:
     *     - enclave quote verification
     *     - public key signature verification
     *     - certificate chain verification
     */
    OE_VERIFY_FAILED,

    /**
     * The function failed to find a resource. Examples of resources include
     * files, directories, and functions (ECALL/OCALL), container elements.
     */
    OE_NOT_FOUND,

    /**
     * The function encountered an overflow in an integer operation, which
     * can occur in arithmetic operations and cast operations.
     */
    OE_INTEGER_OVERFLOW,

    /**
     * The certificate does not contain a public key.
     */
    OE_PUBLIC_KEY_NOT_FOUND,

    /**
     * An integer index is outside the expected range. For example, an array
     * index is greater than or equal to the array size.
     */
    OE_OUT_OF_BOUNDS,

    /**
     * The function prevented an attempt to perform an overlapped copy, where
     * the source and destination buffers are overlapping.
     */
    OE_OVERLAPPED_COPY,

    /**
     * The function detected a constraint failure. A constraint restricts the
     * the value of a field, parameter, or variable. For example, the value of
     * **day_of_the_week** must be between 1 and 7 inclusive.
     */
    OE_CONSTRAINT_FAILED,

    /**
     * An **IOCTL** operation failed. Open Enclave uses **IOCTL** operations to
     * communicate with the Intel SGX driver.
     */
    OE_IOCTL_FAILED,

    /**
     * The given operation is unsupported, usually by a particular platform
     * or environment.
     */
    OE_UNSUPPORTED,

    /**
     * The function failed to read data from a device (such as a socket, or
     * file).
     */
    OE_READ_FAILED,

    /**
     * A software service is unavailable (such as the AESM service).
     */
    OE_SERVICE_UNAVAILABLE,

    /**
     * The operation cannot be completed because the enclave is aborting.
     */
    OE_ENCLAVE_ABORTING,

    /**
     * The operation cannot be completed because the enclave has already
     * aborted.
     */
    OE_ENCLAVE_ABORTED,

    /**
     * The underlying platform or hardware returned an error. For example,
     * an SGX user-mode instruction failed.
     */
    OE_PLATFORM_ERROR,

    /**
     * The given **CPUSVN** value is invalid. An SGX user-mode instruction may
     * return this error.
     */
    OE_INVALID_CPUSVN,

    /**
     * The given **ISVSNV** value is invalid. An SGX user-mode instruction may
     * return this error.
     */
    OE_INVALID_ISVSVN,

    /**
     * The given **key name** is invalid. An SGX user-mode instruction may
     * return this error.
     */
    OE_INVALID_KEYNAME,

    /**
     * Attempted to create a debug enclave with an enclave image that does
     * not allow it.
     */
    OE_DEBUG_DOWNGRADE,

    /**
     * Failed to parse an enclave report.
     */
    OE_REPORT_PARSE_ERROR,

    /**
     * The certificate chain is not available or missing.
     */
    OE_MISSING_CERTIFICATE_CHAIN,

    /**
     * An operation cannot be performed beause the resource is busy. For
     * example, a non-recursive mutex cannot be locked because it is already
     * locked.
     */
    OE_BUSY,

    /**
     * An operation cannot be performed because the requestor is not the
     * owner of the resource. For example, a thread cannot lock a mutex
     * because it is not the thread that acquired the mutex.
     */
    OE_NOT_OWNER,

    /**
     * The certificate does not contain the expected SGX extensions.
     */
    OE_INVALID_SGX_CERTIFICATE_EXTENSIONS,

    /**
     * A memory leak was detected during enclave termination.
     */
    OE_MEMORY_LEAK,

    /**
     * The data is improperly aligned for the given operation. This may occur
     * when an output buffer parameter is not suitably aligned for the data
     * it will receive.
     */
    OE_BAD_ALIGNMENT,

    /**
     * Failed to parse the trusted computing base (TCB) revocation data
     * for the enclave.
     */
    OE_TCB_INFO_PARSE_ERROR,

    /**
     * The level of the trusted computing base (TCB) is not up to date for
     * report verification.
     */
    OE_TCB_LEVEL_INVALID,

    /**
     * Failed to load the quote provider library used for quote generation
     * and attestation.
     */
    OE_QUOTE_PROVIDER_LOAD_ERROR,

    /**
     * A call to the quote provider failed.
     */
    OE_QUOTE_PROVIDER_CALL_ERROR,

    /**
     * The certificate revocation data for attesting the trusted computing
     * base (TCB) is invalid for this enclave.
     */
    OE_INVALID_REVOCATION_INFO,

    /**
     * The given UTC date-time string or structure is invalid. This occurs
     * when (1) an element is out of range (year, month, day, hours, minutes,
     * seconds), or (2) the UTC date-time string is malformed.
     */
    OE_INVALID_UTC_DATE_TIME,

    __OE_RESULT_MAX = OE_ENUM_MAX,
} oe_result_t;

/**
 * Retrieve a string for a result code.
 *
 * This function retrieves a string description for the given **result**
 * parameter.
 *
 * @param result Retrieve string description for this result code.
 *
 * @returns Returns a pointer to a static string description.
 *
 */
const char* oe_result_str(oe_result_t result);

OE_EXTERNC_END

#endif /* _OE_BITS_RESULT_H */
