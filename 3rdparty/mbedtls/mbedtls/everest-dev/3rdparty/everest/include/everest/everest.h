/*
 *  Interface to code from Project Everest
 *
 *  Copyright 2016-2018 INRIA and Microsoft Corporation
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of Mbed TLS (https://tls.mbed.org).
 */
#ifndef MBEDTLS_EVEREST_H
#define MBEDTLS_EVEREST_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/ecp.h"
#include "everest/x25519.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines the source of the imported EC key.
 */
typedef enum
{
    MBEDTLS_EVEREST_ECDH_OURS,   /**< Our key. */
    MBEDTLS_EVEREST_ECDH_THEIRS, /**< The key of the peer. */
} mbedtls_everest_ecdh_side;

typedef struct {
    mbedtls_x25519_context ctx;
} mbedtls_ecdh_context_everest;


/**
 * \brief           This function sets up the ECDH context with the information
 *                  given.
 *
 *                  This function should be called after mbedtls_ecdh_init() but
 *                  before mbedtls_ecdh_make_params(). There is no need to call
 *                  this function before mbedtls_ecdh_read_params().
 *
 *                  This is the first function used by a TLS server for ECDHE
 *                  ciphersuites.
 *
 * \param ctx       The ECDH context to set up.
 * \param grp_id    The group id of the group to set up the context for.
 *
 * \return          \c 0 on success.
 */
int mbedtls_everest_setup( mbedtls_ecdh_context_everest *ctx, int grp_id );

/**
 * \brief           This function frees a context.
 *
 * \param ctx       The context to free.
 */
void mbedtls_everest_free( mbedtls_ecdh_context_everest *ctx );

/**
 * \brief           This function generates a public key and a TLS
 *                  ServerKeyExchange payload.
 *
 *                  This is the second function used by a TLS server for ECDHE
 *                  ciphersuites. (It is called after mbedtls_ecdh_setup().)
 *
 * \note            This function assumes that the ECP group (grp) of the
 *                  \p ctx context has already been properly set,
 *                  for example, using mbedtls_ecp_group_load().
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDH context.
 * \param olen      The number of characters written.
 * \param buf       The destination buffer.
 * \param blen      The length of the destination buffer.
 * \param f_rng     The RNG function.
 * \param p_rng     The RNG context.
 *
 * \return          \c 0 on success.
 * \return          An \c MBEDTLS_ERR_ECP_XXX error code on failure.
 */
int mbedtls_everest_make_params( mbedtls_ecdh_context_everest *ctx, size_t *olen,
                                 unsigned char *buf, size_t blen,
                                 int( *f_rng )( void *, unsigned char *, size_t ),
                                 void *p_rng );

/**
 * \brief           This function parses and processes a TLS ServerKeyExhange
 *                  payload.
 *
 *                  This is the first function used by a TLS client for ECDHE
 *                  ciphersuites.
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDH context.
 * \param buf       The pointer to the start of the input buffer.
 * \param end       The address for one Byte past the end of the buffer.
 *
 * \return          \c 0 on success.
 * \return          An \c MBEDTLS_ERR_ECP_XXX error code on failure.
 *
 */
int mbedtls_everest_read_params( mbedtls_ecdh_context_everest *ctx,
                                 const unsigned char **buf, const unsigned char *end );

/**
 * \brief           This function parses and processes a TLS ServerKeyExhange
 *                  payload.
 *
 *                  This is the first function used by a TLS client for ECDHE
 *                  ciphersuites.
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDH context.
 * \param buf       The pointer to the start of the input buffer.
 * \param end       The address for one Byte past the end of the buffer.
 *
 * \return          \c 0 on success.
 * \return          An \c MBEDTLS_ERR_ECP_XXX error code on failure.
 *
 */
int mbedtls_everest_read_params( mbedtls_ecdh_context_everest *ctx,
                                 const unsigned char **buf, const unsigned char *end );

/**
 * \brief           This function sets up an ECDH context from an EC key.
 *
 *                  It is used by clients and servers in place of the
 *                  ServerKeyEchange for static ECDH, and imports ECDH
 *                  parameters from the EC key information of a certificate.
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDH context to set up.
 * \param key       The EC key to use.
 * \param side      Defines the source of the key: 1: Our key, or
 *                  0: The key of the peer.
 *
 * \return          \c 0 on success.
 * \return          An \c MBEDTLS_ERR_ECP_XXX error code on failure.
 *
 */
int mbedtls_everest_get_params( mbedtls_ecdh_context_everest *ctx, const mbedtls_ecp_keypair *key,
                                mbedtls_everest_ecdh_side side );

/**
 * \brief           This function generates a public key and a TLS
 *                  ClientKeyExchange payload.
 *
 *                  This is the second function used by a TLS client for ECDH(E)
 *                  ciphersuites.
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDH context.
 * \param olen      The number of Bytes written.
 * \param buf       The destination buffer.
 * \param blen      The size of the destination buffer.
 * \param f_rng     The RNG function.
 * \param p_rng     The RNG context.
 *
 * \return          \c 0 on success.
 * \return          An \c MBEDTLS_ERR_ECP_XXX error code on failure.
 */
int mbedtls_everest_make_public( mbedtls_ecdh_context_everest *ctx, size_t *olen,
                                 unsigned char *buf, size_t blen,
                                 int( *f_rng )( void *, unsigned char *, size_t ),
                                 void *p_rng );

/**
 * \brief       This function parses and processes a TLS ClientKeyExchange
 *              payload.
 *
 *              This is the third function used by a TLS server for ECDH(E)
 *              ciphersuites. (It is called after mbedtls_ecdh_setup() and
 *              mbedtls_ecdh_make_params().)
 *
 * \see         ecp.h
 *
 * \param ctx   The ECDH context.
 * \param buf   The start of the input buffer.
 * \param blen  The length of the input buffer.
 *
 * \return      \c 0 on success.
 * \return      An \c MBEDTLS_ERR_ECP_XXX error code on failure.
 */
int mbedtls_everest_read_public( mbedtls_ecdh_context_everest *ctx,
                                 const unsigned char *buf, size_t blen );

/**
 * \brief           This function derives and exports the shared secret.
 *
 *                  This is the last function used by both TLS client
 *                  and servers.
 *
 * \note            If \p f_rng is not NULL, it is used to implement
 *                  countermeasures against side-channel attacks.
 *                  For more information, see mbedtls_ecp_mul().
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDH context.
 * \param olen      The number of Bytes written.
 * \param buf       The destination buffer.
 * \param blen      The length of the destination buffer.
 * \param f_rng     The RNG function.
 * \param p_rng     The RNG context.
 *
 * \return          \c 0 on success.
 * \return          An \c MBEDTLS_ERR_ECP_XXX error code on failure.
 */
int mbedtls_everest_calc_secret( mbedtls_ecdh_context_everest *ctx, size_t *olen,
                                 unsigned char *buf, size_t blen,
                                 int( *f_rng )( void *, unsigned char *, size_t ),
                                 void *p_rng );

#if defined(MBEDTLS_ECDH_VARIANT_EVEREST_AES_GCM)

/* Everest AES-GCM  */

typedef unsigned char everest_byte;

typedef struct {
    everest_byte *plain_ptr;
    uint64_t plain_num_bytes;
    everest_byte *auth_ptr;
    uint64_t auth_num_bytes;
    everest_byte *iv_ptr;
    everest_byte *expanded_key_ptr;
    everest_byte *out_ptr;
    everest_byte *tag_ptr;
} vale_args_t;

typedef struct
{
    vale_args_t vale_args;

    size_t plain_ptr_size;
    size_t auth_ptr_size;
    size_t out_ptr_size;
    size_t tag_ptr_size;
    unsigned char iv_buf[16];
    const unsigned char *key;
} everest_aes_gcm_args;

void everest_aes_gcm_args_init( everest_aes_gcm_args * args, unsigned int keysize );

extern void aes128_key_expansion( everest_byte *key_ptr, everest_byte *key_expansion_ptr );
extern void gcm128_encrypt( vale_args_t *a );
extern int gcm128_decrypt( vale_args_t *a );

extern void aes256_key_expansion( everest_byte *key_ptr, everest_byte *key_expansion_ptr );
extern void gcm256_encrypt( vale_args_t *a );
extern int gcm256_decrypt( vale_args_t *a );

struct mbedtls_gcm_context;
typedef struct mbedtls_gcm_context mbedtls_gcm_context;

void mbedtls_everest_aes_gcm_args_init( everest_aes_gcm_args * args, unsigned int keysize );

int mbedtls_everest_aes_gcm_setkey( mbedtls_gcm_context *ctx,
                                    const unsigned char *key,
                                    unsigned int keybits );

void mbedtls_everest_aes_gcm_wipe_key( mbedtls_gcm_context *ctx );

int mbedtls_everest_aes_gcm_free( mbedtls_gcm_context *ctx);

void mbedtls_everest_prepare_aes_gcm_buffer(
    unsigned char *inbuf, size_t inbuf_size,
    unsigned char **argbuf, size_t *argbuf_size,
    int needs_copy );

void mbedtls_everest_prepare_aes_gcm_buffers(
    everest_aes_gcm_args * args,
    size_t length,
    unsigned char *add, size_t add_len,
    unsigned char *input,
    unsigned char *output,
    size_t tag_len, unsigned char *tag );


#endif

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_EVEREST_H */
