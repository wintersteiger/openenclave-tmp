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

int main(int argc, char** argv)
{
    oe_result_t result;
    DIR* dir;
    struct dirent* entry;
    std::vector<uint8_t> tcbInfo;
    oe_parsed_tcb_info_t parsedInfo;
    oe_tcb_level_t platformTcbLevel = {
        {4, 4, 2, 4, 1, 128, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        8,
        OE_TCB_LEVEL_STATUS_UNKNOWN};
    char buf[1024];
    if (argc != 2)
    {
        fprintf(
            stderr, "Usage: %s << Path to the respective file >> \n", argv[0]);
        exit(1);
    }
    static int count = 0;
    if ((dir = opendir(argv[1])) != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            count++;
            if ((strcmp(entry->d_name, ".") == 0) ||
                (strcmp(entry->d_name, "..") == 0))
                continue;
            if (strstr(entry->d_name, "json") == NULL)
            {
                continue;
            }
            sprintf(buf, "%s/%s", argv[1], entry->d_name);
            tcbInfo = FileToBytes(buf);
            memset((void*)&parsedInfo, '\0', sizeof(oe_parsed_tcb_info_t));
            platformTcbLevel.status = OE_TCB_LEVEL_STATUS_UNKNOWN;

            result = oe_parse_tcb_info_json(
                (uint8_t*)&tcbInfo[0],
                (uint32_t)tcbInfo.size(),
                &platformTcbLevel,
                &parsedInfo);
            printf(
                "\nplatformTcbLevel.status =%d return= %d for %s\n",
                platformTcbLevel.status,
                result,
                entry->d_name);
        }
        closedir(dir);
    }
    else
    {
        /* cannot open the directory */
        assert("INVALID_PATH == TRUE");
    }
    return 0;
}
