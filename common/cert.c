// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/bits/safecrt.h>
#include <openenclave/internal/asn1.h>
#include <openenclave/internal/cert.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/utils.h>
#include "common.h"

static oe_result_t _find_url(
    const uint8_t* data,
    size_t size,
    const char** url,
    size_t* url_len)
{
    oe_result_t result = OE_UNEXPECTED;
    const uint8_t* p = data;
    size_t remaining = size;
    const char pattern[] = "http";
    const size_t pattern_length = sizeof(pattern) - 1;

    /* Search for "http" preceded by the length of the URL */
    while (remaining >= pattern_length)
    {
        if (memcmp(p, pattern, pattern_length) == 0)
        {
            /* Fail if data begins with the pattern */
            if (p == data)
                OE_RAISE(OE_FAILURE);

            /* Get the length which immediately precedes the pattern */
            uint8_t len = p[-1];

            /* Fail if length exceeds bytes remaining in the buffer */
            if (len > remaining)
                OE_RAISE(OE_FAILURE);

            *url = (char*)p;
            *url_len = len;
            result = OE_OK;
            goto done;
        }

        remaining--;
        p++;
    }

    result = OE_FAILURE;

done:
    return result;
}

// Append up to 'n' bytes of string 's' to the buffer at the given offset. If
// less than 'n' bytes remain, then ignore the excess bytes of string 's'.
// Update the offset, which may legally exceed the buffer size. Upon return,
// the offset indicates how many bytes would be required to hold the data.
static void _append(
    void* buffer,
    size_t size,
    size_t* offset,
    const void* s,
    size_t n)
{
    /* If any space remaining in the buffer: */
    if (*offset < size)
    {
        const size_t remaining = size - *offset;
        const size_t m = (remaining < n) ? remaining : n;
        void* ptr = (uint8_t*)buffer + *offset;

        if (s)
        {
            // Copy 'm' bytes from string 's'.
            oe_memcpy_s(ptr, m, s, m);
        }
        else
        {
            // Fill with 'm' zero bytes.
            memset(ptr, 0, m);
        }
    }

    *offset += n;
}

oe_result_t oe_get_crl_distribution_points(
    const oe_cert_t* cert,
    const char*** urls,
    size_t* num_urls,
    uint8_t* buffer,
    size_t* buffer_size)
{
    oe_result_t result = OE_UNEXPECTED;
    size_t size = 0;
    size_t offset = 0;
    static const char _OID[] = "2.5.29.31";

    if (urls)
        *urls = NULL;

    if (num_urls)
        *num_urls = 0;

    if (!cert || !urls || !num_urls || !buffer_size)
        OE_RAISE(OE_INVALID_PARAMETER);

    if (!buffer && *buffer_size != 0)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* If buffer is not aligned properly to hold an array of pointers */
    if (oe_align_pointer(buffer, sizeof(void*)) != buffer)
        OE_RAISE(OE_BAD_ALIGNMENT);

    /* Determine the size of the extension */
    if (oe_cert_find_extension(cert, _OID, NULL, &size) != OE_BUFFER_TOO_SMALL)
        OE_RAISE(OE_FAILURE);

    /* Find all the CRL distribution points in this extension */
    {
        uint8_t data[size];
        oe_asn1_t asn1;
        size_t urls_bytes;

        /* Find the extension */
        size = sizeof(data);
        OE_CHECK(oe_cert_find_extension(cert, _OID, data, &size));

        /* Determine the number of URLs */
        {
            oe_asn1_t seq;

            oe_asn1_init(&asn1, data, size);
            OE_CHECK(oe_asn1_get_sequence(&asn1, &seq));

            while (oe_asn1_more(&seq))
            {
                oe_asn1_t crldp;
                OE_CHECK(oe_asn1_get_sequence(&seq, &crldp));
                (*num_urls)++;
            }
        }

        /* Determine the number of bytes needed by the urls[] array */
        urls_bytes = sizeof(char*) * (*num_urls);

        /* Leave space for urls[] array */
        _append(buffer, *buffer_size, &offset, NULL, urls_bytes);

        /* Set the pointer to the urls[] array if enough space */
        if (buffer && urls_bytes <= *buffer_size)
            *urls = (const char**)buffer;

        /* Process all the CRL distribution points */
        {
            oe_asn1_t seq;

            oe_asn1_init(&asn1, data, size);
            OE_CHECK(oe_asn1_get_sequence(&asn1, &seq));

            /* While there are more CRL distribution points */
            for (size_t i = 0; oe_asn1_more(&seq); i++)
            {
                oe_asn1_t crldp;
                const char* url;
                size_t url_len;

                OE_CHECK(oe_asn1_get_sequence(&seq, &crldp));
                OE_CHECK(_find_url(crldp.data, crldp.length, &url, &url_len));

                /* Append current buffer position to the urls[] array */
                if (*urls)
                {
                    // The address could point beyond end of buffer, but that is
                    // fine since an OE_BUFFER_TOO_SMALL error is raised below.
                    (*urls)[i] = (const char*)(buffer + offset);
                }

                /* Append the URL */
                _append(buffer, *buffer_size, &offset, url, url_len);

                /* Append null terminator */
                _append(buffer, *buffer_size, &offset, NULL, sizeof(char));
            }
        }
    }

    if (offset > *buffer_size)
    {
        *buffer_size = offset;
        OE_RAISE(OE_BUFFER_TOO_SMALL);
    }

    *buffer_size = offset;
    result = OE_OK;

done:
    return result;
}
