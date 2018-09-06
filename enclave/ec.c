// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "ec.h"
#include <mbedtls/asn1.h>
#include <mbedtls/asn1write.h>
#include <mbedtls/ecp.h>
#include <openenclave/enclave.h>
#include <openenclave/internal/enclavelibc.h>
#include <openenclave/internal/raise.h>
#include "key.h"
#include "pem.h"
#include "random.h"

static uint64_t _PRIVATE_KEY_MAGIC = 0xf12c37bb02814eeb;
static uint64_t _PUBLIC_KEY_MAGIC = 0xd7490a56f6504ee6;

OE_STATIC_ASSERT(sizeof(oe_private_key_t) <= sizeof(oe_ec_private_key_t));
OE_STATIC_ASSERT(sizeof(oe_public_key_t) <= sizeof(oe_ec_public_key_t));

static mbedtls_ecp_group_id _GetGroupID(oe_ec_type_t ecType)
{
    switch (ecType)
    {
        case OE_EC_TYPE_SECP256R1:
            return MBEDTLS_ECP_DP_SECP256R1;
        default:
            return MBEDTLS_ECP_DP_NONE;
    }
}

static oe_result_t _CopyKey(
    mbedtls_pk_context* dest,
    const mbedtls_pk_context* src,
    bool copyPrivateFields)
{
    oe_result_t result = OE_UNEXPECTED;
    const mbedtls_pk_info_t* info;

    if (dest)
        mbedtls_pk_init(dest);

    /* Check parameters */
    if (!dest || !src)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Lookup the info for this key type */
    if (!(info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)))
        OE_RAISE(OE_PUBLIC_KEY_NOT_FOUND);

    /* Setup the context for this key type */
    if (mbedtls_pk_setup(dest, info) != 0)
        OE_RAISE(OE_FAILURE);

    /* Copy all fields of the key structure */
    {
        mbedtls_ecp_keypair* ecDest = mbedtls_pk_ec(*dest);
        const mbedtls_ecp_keypair* ecSrc = mbedtls_pk_ec(*src);

        if (!ecDest || !ecSrc)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_ecp_group_copy(&ecDest->grp, &ecSrc->grp) != 0)
            OE_RAISE(OE_FAILURE);

        if (copyPrivateFields)
        {
            if (mbedtls_mpi_copy(&ecDest->d, &ecSrc->d) != 0)
                OE_RAISE(OE_FAILURE);
        }

        if (mbedtls_ecp_copy(&ecDest->Q, &ecSrc->Q) != 0)
            OE_RAISE(OE_FAILURE);
    }

    result = OE_OK;

done:

    if (result != OE_OK)
        mbedtls_pk_free(dest);

    return result;
}

static oe_result_t _GenerateKeyPair(
    oe_ec_type_t ecType,
    oe_private_key_t* privateKey,
    oe_public_key_t* publicKey)
{
    oe_result_t result = OE_UNEXPECTED;
    mbedtls_ctr_drbg_context* drbg;
    mbedtls_pk_context pk;
    int curve;

    /* Initialize structures */
    mbedtls_pk_init(&pk);

    if (privateKey)
        oe_memset(privateKey, 0, sizeof(*privateKey));

    if (publicKey)
        oe_memset(publicKey, 0, sizeof(*publicKey));

    /* Check for invalid parameters */
    if (!privateKey || !publicKey)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Get the group id and curve info structure for this EC type */
    {
        const mbedtls_ecp_curve_info* info;
        mbedtls_ecp_group_id groupID;

        if ((groupID = _GetGroupID(ecType)) == MBEDTLS_ECP_DP_NONE)
            OE_RAISE(OE_FAILURE);

        if (!(info = mbedtls_ecp_curve_info_from_grp_id(groupID)))
            OE_RAISE(OE_INVALID_PARAMETER);

        curve = info->grp_id;
    }

    /* Get the drbg object */
    if (!(drbg = oe_mbedtls_get_drbg()))
        OE_RAISE(OE_FAILURE);

    /* Create key struct */
    if (mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) != 0)
        OE_RAISE(OE_FAILURE);

    /* Generate the EC key */
    if (mbedtls_ecp_gen_key(
            curve, mbedtls_pk_ec(pk), mbedtls_ctr_drbg_random, drbg) != 0)
    {
        OE_RAISE(OE_FAILURE);
    }

    /* Initialize the private key parameter */
    OE_CHECK(
        oe_private_key_init(privateKey, &pk, _CopyKey, _PRIVATE_KEY_MAGIC));

    /* Initialize the public key parameter */
    OE_CHECK(oe_public_key_init(publicKey, &pk, _CopyKey, _PUBLIC_KEY_MAGIC));

    result = OE_OK;

done:

    mbedtls_pk_free(&pk);

    if (result != OE_OK)
    {
        oe_private_key_free(privateKey, _PRIVATE_KEY_MAGIC);
        oe_public_key_free(publicKey, _PUBLIC_KEY_MAGIC);
    }

    return result;
}

static oe_result_t oe_public_key_equal(
    const oe_public_key_t* publicKey1,
    const oe_public_key_t* publicKey2,
    bool* equal)
{
    oe_result_t result = OE_UNEXPECTED;

    if (equal)
        *equal = false;

    /* Reject bad parameters */
    if (!oe_public_key_is_valid(publicKey1, _PUBLIC_KEY_MAGIC) ||
        !oe_public_key_is_valid(publicKey2, _PUBLIC_KEY_MAGIC) || !equal)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Compare the groups and EC points */
    {
        const mbedtls_ecp_keypair* ec1 = mbedtls_pk_ec(publicKey1->pk);
        const mbedtls_ecp_keypair* ec2 = mbedtls_pk_ec(publicKey2->pk);

        if (!ec1 || !ec2)
            OE_RAISE(OE_INVALID_PARAMETER);

        if (ec1->grp.id == ec2->grp.id &&
            mbedtls_ecp_point_cmp(&ec1->Q, &ec2->Q) == 0)
        {
            *equal = true;
        }
    }

    result = OE_OK;

done:
    return result;
}

oe_result_t oe_ec_public_key_init(
    oe_ec_public_key_t* publicKey,
    const mbedtls_pk_context* pk)
{
    return oe_public_key_init(
        (oe_public_key_t*)publicKey, pk, _CopyKey, _PUBLIC_KEY_MAGIC);
}

oe_result_t oe_ec_private_key_read_pem(
    oe_ec_private_key_t* privateKey,
    const uint8_t* pemData,
    size_t pemSize)
{
    return oe_private_key_read_pem(
        pemData,
        pemSize,
        (oe_private_key_t*)privateKey,
        MBEDTLS_PK_ECKEY,
        _PRIVATE_KEY_MAGIC);
}

oe_result_t oe_ec_private_key_write_pem(
    const oe_ec_private_key_t* privateKey,
    uint8_t* pemData,
    size_t* pemSize)
{
    return oe_private_key_write_pem(
        (const oe_private_key_t*)privateKey,
        pemData,
        pemSize,
        _PRIVATE_KEY_MAGIC);
}

oe_result_t oe_ec_public_key_read_pem(
    oe_ec_public_key_t* privateKey,
    const uint8_t* pemData,
    size_t pemSize)
{
    return oe_public_key_read_pem(
        pemData,
        pemSize,
        (oe_public_key_t*)privateKey,
        MBEDTLS_PK_ECKEY,
        _PUBLIC_KEY_MAGIC);
}

oe_result_t oe_ec_public_key_write_pem(
    const oe_ec_public_key_t* privateKey,
    uint8_t* pemData,
    size_t* pemSize)
{
    return oe_public_key_write_pem(
        (const oe_public_key_t*)privateKey,
        pemData,
        pemSize,
        _PUBLIC_KEY_MAGIC);
}

oe_result_t oe_ec_private_key_free(oe_ec_private_key_t* privateKey)
{
    return oe_private_key_free(
        (oe_private_key_t*)privateKey, _PRIVATE_KEY_MAGIC);
}

oe_result_t oe_ec_public_key_free(oe_ec_public_key_t* publicKey)
{
    return oe_public_key_free((oe_public_key_t*)publicKey, _PUBLIC_KEY_MAGIC);
}

oe_result_t oe_ec_private_key_sign(
    const oe_ec_private_key_t* privateKey,
    oe_hash_type_t hashType,
    const void* hashData,
    size_t hashSize,
    uint8_t* signature,
    size_t* signatureSize)
{
    return oe_private_key_sign(
        (oe_private_key_t*)privateKey,
        hashType,
        hashData,
        hashSize,
        signature,
        signatureSize,
        _PRIVATE_KEY_MAGIC);
}

oe_result_t oe_ec_public_key_verify(
    const oe_ec_public_key_t* publicKey,
    oe_hash_type_t hashType,
    const void* hashData,
    size_t hashSize,
    const uint8_t* signature,
    size_t signatureSize)
{
    return oe_public_key_verify(
        (oe_public_key_t*)publicKey,
        hashType,
        hashData,
        hashSize,
        signature,
        signatureSize,
        _PUBLIC_KEY_MAGIC);
}

oe_result_t oe_ec_generate_key_pair(
    oe_ec_type_t type,
    oe_ec_private_key_t* privateKey,
    oe_ec_public_key_t* publicKey)
{
    return _GenerateKeyPair(
        type, (oe_private_key_t*)privateKey, (oe_public_key_t*)publicKey);
}

oe_result_t oe_ec_public_key_equal(
    const oe_ec_public_key_t* publicKey1,
    const oe_ec_public_key_t* publicKey2,
    bool* equal)
{
    return oe_public_key_equal(
        (oe_public_key_t*)publicKey1, (oe_public_key_t*)publicKey2, equal);
}

oe_result_t oe_ec_public_key_from_coordinates(
    oe_ec_public_key_t* publicKey,
    oe_ec_type_t ecType,
    const uint8_t* xData,
    size_t xSize,
    const uint8_t* yData,
    size_t ySize)
{
    oe_result_t result = OE_UNEXPECTED;
    oe_public_key_t* impl = (oe_public_key_t*)publicKey;
    const mbedtls_pk_info_t* info;

    if (publicKey)
        oe_memset(publicKey, 0, sizeof(oe_ec_public_key_t));

    if (impl)
        mbedtls_pk_init(&impl->pk);

    /* Reject invalid parameters */
    if (!publicKey || !xData || !xSize || !yData || !ySize)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Lookup the info for this key type */
    if (!(info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)))
        OE_RAISE(OE_PUBLIC_KEY_NOT_FOUND);

    /* Setup the context for this key type */
    if (mbedtls_pk_setup(&impl->pk, info) != 0)
        OE_RAISE(OE_FAILURE);

    /* Initialize the key */
    {
        mbedtls_ecp_keypair* ecp = mbedtls_pk_ec(impl->pk);
        mbedtls_ecp_group_id groupID;

        if ((groupID = _GetGroupID(ecType)) == MBEDTLS_ECP_DP_NONE)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_ecp_group_load(&ecp->grp, groupID) != 0)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_mpi_read_binary(&ecp->Q.X, xData, xSize) != 0)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_mpi_read_binary(&ecp->Q.Y, yData, ySize) != 0)
            OE_RAISE(OE_FAILURE);

        // Used internally by MBEDTLS. Set Z to 1 to indicate that X-Y
        // represents a standard coordinate point. Zero indicates that the
        // point is zero or infinite, and values >= 2 have internal meaning
        // only to MBEDTLS.
        if (mbedtls_mpi_lset(&ecp->Q.Z, 1) != 0)
            OE_RAISE(OE_FAILURE);
    }

    /* Set the magic number */
    impl->magic = _PUBLIC_KEY_MAGIC;

    result = OE_OK;

done:

    if (result != OE_OK && impl)
        mbedtls_pk_free(&impl->pk);

    return result;
}

oe_result_t oe_ecdsa_signature_write_der(
    unsigned char* signature,
    size_t* signatureSize,
    const uint8_t* rData,
    size_t rSize,
    const uint8_t* sData,
    size_t sSize)
{
    oe_result_t result = OE_UNEXPECTED;
    mbedtls_mpi r;
    mbedtls_mpi s;
    unsigned char buf[MBEDTLS_ECDSA_MAX_LEN];
    unsigned char* p = buf + sizeof(buf);
    int n;
    size_t len = 0;

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    /* Reject invalid parameters */
    if (!signatureSize || !rData || !rSize || !sData || !sSize)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* If signature is null, then signatureSize must be zero */
    if (!signature && *signatureSize != 0)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Convert raw R data to big number */
    if (mbedtls_mpi_read_binary(&r, rData, rSize) != 0)
        OE_RAISE(OE_FAILURE);

    /* Convert raw S data to big number */
    if (mbedtls_mpi_read_binary(&s, sData, sSize) != 0)
        OE_RAISE(OE_FAILURE);

    /* Write S to ASN.1 */
    {
        if ((n = mbedtls_asn1_write_mpi(&p, buf, &s)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Write R to ASN.1 */
    {
        if ((n = mbedtls_asn1_write_mpi(&p, buf, &r)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Write the length to ASN.1 */
    {
        if ((n = mbedtls_asn1_write_len(&p, buf, len)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Write the tag to ASN.1 */
    {
        unsigned char tag = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;

        if ((n = mbedtls_asn1_write_tag(&p, buf, tag)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Check that buffer is big enough */
    if (len > *signatureSize)
    {
        *signatureSize = len;
        OE_RAISE(OE_BUFFER_TOO_SMALL);
    }

    oe_memcpy(signature, p, len);
    *signatureSize = len;

    result = OE_OK;

done:

    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);

    return result;
}
