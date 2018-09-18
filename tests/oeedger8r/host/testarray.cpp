// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "../edltestutils.h"

#include <openenclave/host.h>
#include <openenclave/internal/tests.h>
#include <algorithm>
#include "array_u.c"

template <typename T>
void init_arrays(T (&a1)[2], T (&a2)[2][2], T (&a3)[3][3], T (&a4)[4][4])
{
    for (size_t i = 0; i < 2; ++i)
        ((T*)a1)[i] = i + 1;

    for (size_t i = 0; i < 4; ++i)
        ((T*)a2)[i] = i + 1;

    for (size_t i = 0; i < 9; ++i)
        ((T*)a3)[i] = i + 1;

    for (size_t i = 0; i < 16; ++i)
        ((T*)a4)[i] = i + 1;
}

template <typename T, typename F>
void test_ecall_array_fun(oe_enclave_t* enclave, F ecall_array_fun)
{
    T a1[2];
    T a2[2][2];
    T a3[3][3];
    T a4[4][4];

    init_arrays(a1, a2, a3, a4);
    OE_TEST(ecall_array_fun(enclave, a1, a2, a3, a4) == OE_OK);
    {
        T exp[] = {4, 3, 2, 1};
        OE_TEST(memcmp(exp, a2, sizeof(a2)) == 0);
    }

    {
        T exp[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
        OE_TEST(memcmp(exp, a3, sizeof(a3)) == 0);
    }
    {
        T exp[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        OE_TEST(memcmp(exp, a4, sizeof(a4)) == 0);
    }

    // Call with nulls.
    OE_TEST(ecall_array_fun(enclave, NULL, NULL, NULL, NULL) == OE_OK);
}

void test_array_edl_ecalls(oe_enclave_t* enclave)
{
    test_ecall_array_fun<char>(enclave, ecall_array_char);
    test_ecall_array_fun<short>(enclave, ecall_array_short);
    test_ecall_array_fun<int>(enclave, ecall_array_int);
    test_ecall_array_fun<float>(enclave, ecall_array_float);
    test_ecall_array_fun<double>(enclave, ecall_array_double);
    test_ecall_array_fun<long>(enclave, ecall_array_long);
    test_ecall_array_fun<size_t>(enclave, ecall_array_size_t);
    test_ecall_array_fun<unsigned>(enclave, ecall_array_unsigned);
    test_ecall_array_fun<int8_t>(enclave, ecall_array_int8_t);
    test_ecall_array_fun<int16_t>(enclave, ecall_array_int16_t);
    test_ecall_array_fun<int32_t>(enclave, ecall_array_int32_t);
    test_ecall_array_fun<int64_t>(enclave, ecall_array_int64_t);
    test_ecall_array_fun<uint8_t>(enclave, ecall_array_uint8_t);
    test_ecall_array_fun<uint16_t>(enclave, ecall_array_uint16_t);
    test_ecall_array_fun<uint32_t>(enclave, ecall_array_uint32_t);
    test_ecall_array_fun<uint64_t>(enclave, ecall_array_uint64_t);

    OE_TEST(ecall_array_assert_all_called(enclave) == OE_OK);
    printf("=== test_array_edl_ecalls passed\n");
}

template <typename T>
void reverse(T* arr, size_t len)
{
    for (size_t i = 0; i < len / 2; ++i)
        std::swap(arr[i], arr[len - 1 - i]);
}

static int num_ocalls = 0;

template <typename T>
void ocall_array_fun_impl(T a1[2], T a2[2][2], T a3[3][3], T a4[4][4])
{
    ++num_ocalls;

    // in
    if (a1)
    {
        OE_TEST(a1[0] == 1 && a1[1] == 2);
    }

    // in-out
    if (a2)
    {
        T exp[] = {1, 2, 3, 4};
        OE_TEST(memcmp(exp, a2, sizeof(T) * 2 * 2) == 0);
        reverse((T*)a2, 4);
    }

    // out
    if (a3)
    {
        for (int i = 0; i < 9; ++i)
        {
            ((T*)a3)[i] = i;
        }
    }

    // user-check
    if (a4)
    {
        // Cannot be accessed on the host side.
    }
}

void ocall_array_char(char a1[2], char a2[2][2], char a3[3][3], char a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_short(
    short a1[2],
    short a2[2][2],
    short a3[3][3],
    short a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_int(int a1[2], int a2[2][2], int a3[3][3], int a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_float(
    float a1[2],
    float a2[2][2],
    float a3[3][3],
    float a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_double(
    double a1[2],
    double a2[2][2],
    double a3[3][3],
    double a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_long(long a1[2], long a2[2][2], long a3[3][3], long a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_size_t(
    size_t a1[2],
    size_t a2[2][2],
    size_t a3[3][3],
    size_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_unsigned(
    unsigned a1[2],
    unsigned a2[2][2],
    unsigned a3[3][3],
    unsigned a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_int8_t(
    int8_t a1[2],
    int8_t a2[2][2],
    int8_t a3[3][3],
    int8_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_int16_t(
    int16_t a1[2],
    int16_t a2[2][2],
    int16_t a3[3][3],
    int16_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_int32_t(
    int32_t a1[2],
    int32_t a2[2][2],
    int32_t a3[3][3],
    int32_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_int64_t(
    int64_t a1[2],
    int64_t a2[2][2],
    int64_t a3[3][3],
    int64_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_uint8_t(
    uint8_t a1[2],
    uint8_t a2[2][2],
    uint8_t a3[3][3],
    uint8_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_uint16_t(
    uint16_t a1[2],
    uint16_t a2[2][2],
    uint16_t a3[3][3],
    uint16_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_uint32_t(
    uint32_t a1[2],
    uint32_t a2[2][2],
    uint32_t a3[3][3],
    uint32_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_uint64_t(
    uint64_t a1[2],
    uint64_t a2[2][2],
    uint64_t a3[3][3],
    uint64_t a4[4][4])
{
    ocall_array_fun_impl(a1, a2, a3, a4);
}

void ocall_array_assert_all_called()
{
    // Each of the 16 functions above is called twice.
    // Once with arrays and then with nulls.
    OE_TEST(num_ocalls == 32);
}
