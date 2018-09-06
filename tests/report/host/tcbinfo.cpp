// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#ifdef OE_USE_LIBSGX

#include <openenclave/host.h>
#include <openenclave/internal/aesm.h>
#include <openenclave/internal/datetime.h>
#include <openenclave/internal/error.h>
#include <openenclave/internal/hexdump.h>
#include <openenclave/internal/tests.h>
#include <openenclave/internal/utils.h>

#include <fstream>
#include <streambuf>
#include <vector>
#include "../../../common/tcbinfo.h"
#include "../../../host/quote.h"
#include "../common/args.h"

#define SKIP_RETURN_CODE 2

std::vector<uint8_t> FileToBytes(const char* path)
{
    std::ifstream f(path, std::ios::binary);
    std::vector<uint8_t> bytes = std::vector<uint8_t>(
        std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());

    if (bytes.empty())
    {
        printf("File %s not found\n", path);
        exit(1);
    }

    // Explicitly add null character so that the bytes can be printed out
    // safely as a string if needed.
    bytes.push_back('\0');
    return bytes;
}

void AssertParsedValues(oe_parsed_tcb_info_t& parsedInfo)
{
    OE_TEST(parsedInfo.version == 1);

    oe_datetime_t expectedIssueDate = {2018, 6, 6, 10, 12, 17};
    OE_TEST(
        oe_datetime_compare(&parsedInfo.issue_date, &expectedIssueDate) == 0);

    uint8_t expectedFmSpc[6] = {0x00, 0x90, 0x6E, 0xA1, 0x00, 0x00};
    OE_TEST(
        memcmp(parsedInfo.fmspc, expectedFmSpc, sizeof(expectedFmSpc)) == 0);

    const uint8_t expectedSignature[] = {
        0x62, 0xd1, 0x81, 0xc4, 0xba, 0x86, 0x32, 0x13, 0xb8, 0x25, 0xd1,
        0xc0, 0xb6, 0x6b, 0x92, 0xa3, 0xdb, 0xdb, 0x27, 0xb8, 0xff, 0x7c,
        0x72, 0x50, 0xcb, 0x2b, 0x2a, 0xb8, 0x7a, 0x8f, 0x90, 0xd5, 0xe5,
        0xa1, 0x41, 0x69, 0x14, 0x36, 0x9d, 0x8f, 0x82, 0xc5, 0x6c, 0xd3,
        0xd8, 0x75, 0xca, 0xa5, 0x4a, 0xe4, 0xb9, 0x17, 0xca, 0xf4, 0xaf,
        0x7a, 0x93, 0xde, 0xc5, 0x20, 0x67, 0xcb, 0xfd, 0x7b};
    OE_TEST(
        memcmp(
            parsedInfo.signature,
            expectedSignature,
            sizeof(expectedSignature)) == 0);
}

void TestVerifyTCBInfo(
    oe_enclave_t* enclave,
    oe_tcb_level_t* platformTcbLevel,
    oe_result_t expected)
{
    std::vector<uint8_t> tcbInfo = FileToBytes("./data/tcbInfo.json");
    oe_parsed_tcb_info_t parsedInfo = {0};
    VerifyTCBInfoArgs args = {
        &tcbInfo[0], (uint32_t)tcbInfo.size(), platformTcbLevel, &parsedInfo};

    OE_TEST(oe_call_enclave(enclave, "TestVerifyTCBInfo", &args) == OE_OK);
    AssertParsedValues(parsedInfo);
    OE_TEST(
        oe_datetime_is_valid(&parsedInfo.next_update) ==
        OE_INVALID_UTC_DATE_TIME);

    // Contains nextUpdate field.
    tcbInfo = FileToBytes("./data/tcbInfo1.json");
    memset(&parsedInfo, 0, sizeof(parsedInfo));
    platformTcbLevel->status = OE_TCB_LEVEL_STATUS_UNKNOWN;
    VerifyTCBInfoArgs args1 = {
        &tcbInfo[0], (uint32_t)tcbInfo.size(), platformTcbLevel, &parsedInfo};
    OE_TEST(oe_call_enclave(enclave, "TestVerifyTCBInfo", &args1) == OE_OK);
    AssertParsedValues(parsedInfo);

    oe_datetime_t nextUpdate = {2019, 6, 6, 10, 12, 17};
    OE_TEST(oe_datetime_compare(&parsedInfo.next_update, &nextUpdate) == 0);
}

void TestVerifyTCBInfo(oe_enclave_t* enclave)
{
    oe_tcb_level_t platformTcbLevel = {
        {4, 4, 2, 4, 1, 128, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        8,
        OE_TCB_LEVEL_STATUS_UNKNOWN};

    // ./data/tcbInfo.json contains 4 tcb levels.
    // The first level with pce svn = 5 is up to date.
    // The second level with pce svn = 4 needs configuration.
    // The third level with pce svn = 3 is out of date.
    // The fourth level with pce svn = 2 is revoked.

    // Set platform pce svn to 8 and assert that
    // the determined status is up to date.
    platformTcbLevel.status = OE_TCB_LEVEL_STATUS_UNKNOWN;
    platformTcbLevel.pce_svn = 8;
    TestVerifyTCBInfo(enclave, &platformTcbLevel, OE_OK);
    OE_TEST(platformTcbLevel.status == OE_TCB_LEVEL_STATUS_UP_TO_DATE);
    printf("UptoDate TCB Level determination test passed.\n");

    // Set platform pce svn to 4 and assert that
    // the determined status is configuration needed.
    platformTcbLevel.status = OE_TCB_LEVEL_STATUS_UNKNOWN;
    platformTcbLevel.pce_svn = 4;
    TestVerifyTCBInfo(enclave, &platformTcbLevel, OE_TCB_LEVEL_INVALID);
    OE_TEST(
        platformTcbLevel.status == OE_TCB_LEVEL_STATUS_CONFIGURATION_NEEDED);
    printf("ConfigurationNeeded TCB Level determination test passed.\n");

    // Set platform pce svn to 3 and assert that
    // the determined status is out of date.
    platformTcbLevel.status = OE_TCB_LEVEL_STATUS_UNKNOWN;
    platformTcbLevel.pce_svn = 3;
    TestVerifyTCBInfo(enclave, &platformTcbLevel, OE_TCB_LEVEL_INVALID);
    OE_TEST(platformTcbLevel.status == OE_TCB_LEVEL_STATUS_OUT_OF_DATE);
    printf("OutOfDate TCB Level determination test passed.\n");

    // Set platform pce svn to 2 and assert that
    // the determined status is revoked.
    platformTcbLevel.status = OE_TCB_LEVEL_STATUS_UNKNOWN;
    platformTcbLevel.pce_svn = 2;
    TestVerifyTCBInfo(enclave, &platformTcbLevel, OE_TCB_LEVEL_INVALID);
    OE_TEST(platformTcbLevel.status == OE_TCB_LEVEL_STATUS_REVOKED);
    printf("OutOfDate TCB Level determination test passed.\n");

    // Set each of the fields to a value not listed in the json and
    // test that the determined status is OE_TCB_LEVEL_INVALID
    for (uint32_t i = 0; i < OE_COUNTOF(platformTcbLevel.sgx_tcb_comp_svn); ++i)
    {
        platformTcbLevel.status = OE_TCB_LEVEL_STATUS_UNKNOWN;
        platformTcbLevel.sgx_tcb_comp_svn[i] = 0;
        TestVerifyTCBInfo(enclave, &platformTcbLevel, OE_TCB_LEVEL_INVALID);
        OE_TEST(platformTcbLevel.status == OE_TCB_LEVEL_STATUS_UNKNOWN);
        platformTcbLevel.sgx_tcb_comp_svn[i] = 1;
    }
    printf("Unknown TCB Level determination test passed.\n");

    printf("TestVerifyTCBInfo: Positive Tests passed\n");

    const char* negativeFiles[] = {
        // In the following files, a property in corresponding level has been
        // capitalized. JSON is case sensitive and therefore schema validation
        // should fail.
        "./data/tcbInfoNegativePropertyMissingLevel0.json",
        "./data/tcbInfoNegativePropertyMissingLevel1.json",
        "./data/tcbInfoNegativePropertyMissingLevel2.json",
        "./data/tcbInfoNegativePropertyMissingLevel3.json",
        // In the following files, a property in corresponding level has wrong
        // type.
        "./data/tcbInfoNegativePropertyWrongTypeLevel0.json",
        "./data/tcbInfoNegativePropertyWrongTypeLevel1.json",
        "./data/tcbInfoNegativePropertyWrongTypeLevel2.json",
        "./data/tcbInfoNegativePropertyWrongTypeLevel3.json",

        // Comp Svn greater than uint8_t
        "./data/tcbInfoNegativeCompSvn.json",

        // pce Svn greater than uint16_t
        "./data/tcbInfoNegativePceSvn.json",

        // Invalid issueDate field.
        "./data/tcbInfoNegativeInvalidIssueDate.json",

        // Invalid nextUpdate field.
        "./data/tcbInfoNegativeInvalidNextUpdate.json",

        // Signature != 64 bytes
        "./data/tcbInfoNegativeSignature.json",

        // Unsupported JSON constructs
        "./data/tcbInfoNegativeStringEscape.json",
        "./data/tcbInfoNegativeIntegerOverflow.json",
        "./data/tcbInfoNegativeIntegerWithSign.json",
        "./data/tcbInfoNegativeFloat.json",
    };

    for (size_t i = 0; i < sizeof(negativeFiles) / sizeof(negativeFiles[0]);
         ++i)
    {
        std::vector<uint8_t> tcbInfo = FileToBytes(negativeFiles[i]);
        oe_parsed_tcb_info_t parsedInfo = {0};
        oe_tcb_level_t platformTcbLevel = {{0}};
        VerifyTCBInfoArgs args = {&tcbInfo[0],
                                  (uint32_t)tcbInfo.size(),
                                  &platformTcbLevel,
                                  &parsedInfo};
        OE_TEST(
            oe_call_enclave(enclave, "TestVerifyTCBInfo", &args) == OE_OK &&
            args.result == OE_TCB_INFO_PARSE_ERROR);
        printf(
            "TestVerifyTCBInfo: Negative Test %s passed\n", negativeFiles[i]);
    }
}

#endif
