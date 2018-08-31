// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <vector>

#include "../../common/tcbinfo.h"

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

    bytes.push_back('\0');
    return bytes;
}
const char* plattcblevel_status_str(int status)
{
    switch (status)
    {
        case OE_TCB_LEVEL_STATUS_UNKNOWN:
            return "OE_TCB_LEVEL_STATUS_UNKNOWN";
        case OE_TCB_LEVEL_STATUS_REVOKED:
            return "OE_TCB_LEVEL_STATUS_REVOKED";
        case OE_TCB_LEVEL_STATUS_OUT_OF_DATE:
            return "OE_TCB_LEVEL_STATUS_OUT_OF_DATE";
        case OE_TCB_LEVEL_STATUS_UP_TO_DATE:
            return "OE_TCB_LEVEL_STATUS_UP_TO_DATE";
        case __OE_TCB_LEVEL_MAX:
            break;
    }
    return "UNKNOWN";
}
int main(int argc, char** argv)
{
    oe_result_t result;
    std::vector<uint8_t> tcbInfo;
    oe_parsed_tcb_info_t parsedInfo;
    oe_tcb_level_t platformTcbLevel = {
        {4, 4, 2, 4, 1, 128, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        8,
        OE_TCB_LEVEL_STATUS_UNKNOWN};
    if (argc != 2)
    {
        fprintf(
            stderr, "Usage: %s << Path to the respective file >> \n", argv[0]);
        exit(1);
    }

    tcbInfo = FileToBytes(argv[1]);

    result = oe_parse_tcb_info_json(
        (uint8_t*)&tcbInfo[0],
        (uint32_t)tcbInfo.size(),
        &platformTcbLevel,
        &parsedInfo);

    printf(
        "\n Json file Platform TCB LEVEL status = %s\n oe_parse_tcb_info_json "
        "return = %s\n",
        plattcblevel_status_str(platformTcbLevel.status),
        oe_result_str(result));
    return 0;
}
