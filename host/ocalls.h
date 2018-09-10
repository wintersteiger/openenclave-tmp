// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_HOST_OCALLS_H
#define _OE_HOST_OCALLS_H

#include "enclave.h"

void HandlePrint(uint64_t argIn);

void HandleMalloc(uint64_t argIn, uint64_t* argOut);
void HandleRealloc(uint64_t argIn, uint64_t* argOut);
void HandleFree(uint64_t arg);

void HandleThreadWait(oe_enclave_t* enclave, uint64_t arg);
void HandleThreadWake(oe_enclave_t* enclave, uint64_t arg);
void HandleThreadWakeWait(oe_enclave_t* enclave, uint64_t argIn);

void HandleGetQuote(uint64_t argIn);
void HandleGetQETargetInfo(uint64_t argIn);
void HandleGetQuoteRevocationInfo(uint64_t argIn);

void oe_handle_sleep(uint64_t argIn);

void oe_handle_get_time(uint64_t arg_in, uint64_t* arg_out);

void oe_handle_backtrace_symbols(oe_enclave_t* enclave, uint64_t arg);

#endif /* _OE_HOST_OCALLS_H */
