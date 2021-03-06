/*
 * Copyright 2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#pragma once

#if defined(__APPLE__) && defined(__MACH__)

#define COMMON_DIGEST_FOR_OPENSSL
#include <CommonCrypto/CommonDigest.h>
#define SHA1 CC_SHA1
#define SHA224 CC_SHA224
#define SHA256 CC_SHA256
#define SHA384 CC_SHA384
#define SHA512 CC_SHA512

#include <CommonCrypto/CommonHMAC.h>
#define HMAC CCHmac

#else

#include <openssl/md5.h>
#include <openssl/sha.h>

#endif

#include "s2n_evp.h"

#include <openssl/evp.h>

#include <stdint.h>

#define S2N_MAX_DIGEST_LEN SHA512_DIGEST_LENGTH

typedef enum {
    S2N_HASH_NONE=0,
    S2N_HASH_MD5,
    S2N_HASH_SHA1,
    S2N_HASH_SHA224,
    S2N_HASH_SHA256,
    S2N_HASH_SHA384,
    S2N_HASH_SHA512,
    S2N_HASH_MD5_SHA1,
    /* Don't add any hash algorithms below S2N_HASH_SENTINEL */
    S2N_HASH_SENTINEL
} s2n_hash_algorithm;

/* The low_level_digest stores all OpenSSL structs that are alg-specific to be used with OpenSSL's low-level hash API's. */
union s2n_hash_low_level_digest {
    MD5_CTX md5;
    SHA_CTX sha1;
    SHA256_CTX sha224;
    SHA256_CTX sha256;
    SHA512_CTX sha384;
    SHA512_CTX sha512;
    struct {
        MD5_CTX md5;
        SHA_CTX sha1;
    } md5_sha1;
};

/* The evp_digest stores all OpenSSL structs to be used with OpenSSL's EVP hash API's. */
struct s2n_hash_evp_digest {
    struct s2n_evp_digest evp;
    /* Always store a secondary evp_digest to allow resetting a hash_state to MD5_SHA1 from another alg. */
    struct s2n_evp_digest evp_md5_secondary;
};

/* s2n_hash_state stores the s2n_hash implementation being used (low-level or EVP),
 * the hash algorithm being used at the time, and either low_level or high_level (EVP) OpenSSL digest structs.
 */
struct s2n_hash_state {
    const struct s2n_hash *hash_impl;
    s2n_hash_algorithm alg;
    union {
        union s2n_hash_low_level_digest low_level;
        struct s2n_hash_evp_digest high_level;
    } digest;
};

/* The s2n hash implementation is abstracted to allow for separate implementations, using
 * either OpenSSL's low-level algorithm-specific API's or OpenSSL's EVP API's.
 */
struct s2n_hash {
    int (*new) (struct s2n_hash_state *state);
    int (*allow_md5_for_fips) (struct s2n_hash_state *state);
    int (*init) (struct s2n_hash_state *state, s2n_hash_algorithm alg);
    int (*update) (struct s2n_hash_state *state, const void *data, uint32_t size);
    int (*digest) (struct s2n_hash_state *state, void *out, uint32_t size);
    int (*copy) (struct s2n_hash_state *to, struct s2n_hash_state *from);
    int (*reset) (struct s2n_hash_state *state);
    int (*free) (struct s2n_hash_state *state);
};

extern int s2n_hash_digest_size(s2n_hash_algorithm alg, uint8_t *out);
extern int s2n_hash_is_available(s2n_hash_algorithm alg);
extern int s2n_hash_new(struct s2n_hash_state *state);
extern int s2n_hash_allow_md5_for_fips(struct s2n_hash_state *state);
extern int s2n_hash_init(struct s2n_hash_state *state, s2n_hash_algorithm alg);
extern int s2n_hash_update(struct s2n_hash_state *state, const void *data, uint32_t size);
extern int s2n_hash_digest(struct s2n_hash_state *state, void *out, uint32_t size);
extern int s2n_hash_copy(struct s2n_hash_state *to, struct s2n_hash_state *from);
extern int s2n_hash_reset(struct s2n_hash_state *state);
extern int s2n_hash_free(struct s2n_hash_state *state);
