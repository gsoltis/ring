/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.] */

#include <openssl/rsa.h>

#include <limits.h>
#include <string.h>

#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/mem.h>
#include <openssl/nid.h>

#include "internal.h"
#include "../internal.h"


RSA *rsa_new_begin(void) {
  RSA *rsa = OPENSSL_malloc(sizeof(RSA));
  if (rsa == NULL) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
    return NULL;
  }

  memset(rsa, 0, sizeof(RSA));

  rsa->flags = RSA_FLAG_CACHE_PUBLIC | RSA_FLAG_CACHE_PRIVATE;
  return rsa;
}

int rsa_new_end(RSA *rsa, BN_CTX *ctx) {
  assert(ctx != NULL);

  assert(rsa->n != NULL);
  assert(rsa->e != NULL);

  assert(rsa->d != NULL);
  assert(BN_get_flags(rsa->d, BN_FLG_CONSTTIME));

  assert(rsa->p != NULL);
  assert(BN_get_flags(rsa->p, BN_FLG_CONSTTIME));

  assert(rsa->q != NULL);
  assert(BN_get_flags(rsa->q, BN_FLG_CONSTTIME));

  assert(rsa->dmp1 != NULL);
  assert(BN_get_flags(rsa->dmp1, BN_FLG_CONSTTIME));

  assert(rsa->dmq1 != NULL);
  assert(BN_get_flags(rsa->dmq1, BN_FLG_CONSTTIME));

  assert(rsa->iqmp != NULL);
  assert(BN_get_flags(rsa->iqmp, BN_FLG_CONSTTIME));

  int ret = 0;

  BIGNUM qq;
  BN_init(&qq);
  BN_set_flags(&qq, BN_FLG_CONSTTIME);

  rsa->mont_n = BN_MONT_CTX_new();
  rsa->mont_p = BN_MONT_CTX_new();
  rsa->mont_q = BN_MONT_CTX_new();
  rsa->mont_qq = BN_MONT_CTX_new();
  rsa->qmn_mont = BN_new();
  rsa->iqmp_mont = BN_new();
  if (rsa->mont_n == NULL ||
      rsa->mont_p == NULL ||
      rsa->mont_q == NULL ||
      rsa->mont_q == NULL ||
      rsa->mont_qq == NULL ||
      rsa->qmn_mont == NULL ||
      rsa->iqmp_mont == NULL ||
      !BN_MONT_CTX_set(rsa->mont_n, rsa->n, ctx) ||
      !BN_MONT_CTX_set(rsa->mont_p, rsa->p, ctx) ||
      !BN_MONT_CTX_set(rsa->mont_q, rsa->q, ctx) ||
      !BN_mod_mul_montgomery(&qq, rsa->q, rsa->q, rsa->mont_n, ctx) ||
      !BN_to_montgomery(&qq, &qq, rsa->mont_n, ctx) ||
      !BN_MONT_CTX_set(rsa->mont_qq, &qq, ctx) ||
      !BN_to_montgomery(rsa->qmn_mont, rsa->q, rsa->mont_n, ctx) ||
      !BN_to_montgomery(rsa->iqmp_mont, rsa->iqmp, rsa->mont_p, ctx)) {
    goto err;
  }

  ret = RSA_check_key(rsa, ctx);

err:
  BN_free(&qq);
  return ret;
}

void RSA_free(RSA *rsa) {
  if (rsa == NULL) {
    return;
  }

  BN_free(rsa->n);
  BN_free(rsa->e);
  BN_free(rsa->d);
  BN_free(rsa->p);
  BN_free(rsa->q);
  BN_free(rsa->dmp1);
  BN_free(rsa->dmq1);
  BN_free(rsa->iqmp);
  BN_MONT_CTX_free(rsa->mont_n);
  BN_MONT_CTX_free(rsa->mont_p);
  BN_MONT_CTX_free(rsa->mont_q);
  BN_MONT_CTX_free(rsa->mont_qq);
  BN_free(rsa->qmn_mont);
  BN_free(rsa->iqmp_mont);
  OPENSSL_free(rsa);
}

/* pkcs1_sig_prefix contains the ASN.1, DER encoded prefix for a hash that is
 * to be signed with PKCS#1. */
struct pkcs1_sig_prefix {
  /* nid identifies the hash function. */
  int nid;
  /* len is the number of bytes of |bytes| which are valid. */
  uint8_t len;
  /* bytes contains the DER bytes. */
  uint8_t bytes[19];
};

/* kPKCS1SigPrefixes contains the ASN.1 prefixes for PKCS#1 signatures with
 * different hash functions. */
static const struct pkcs1_sig_prefix kPKCS1SigPrefixes[] = {
    {
     NID_sha1,
     15,
     {0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 0x1a, 0x05,
      0x00, 0x04, 0x14},
    },
    {
     NID_sha256,
     19,
     {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03,
      0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20},
    },
    {
     NID_sha384,
     19,
     {0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03,
      0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30},
    },
    {
     NID_sha512,
     19,
     {0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03,
      0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40},
    },
    {
     NID_undef, 0, {0},
    },
};

int RSA_add_pkcs1_prefix(uint8_t **out_msg, size_t *out_msg_len, int hash_nid,
                         const uint8_t *msg, size_t msg_len) {
  unsigned i;

  for (i = 0; kPKCS1SigPrefixes[i].nid != NID_undef; i++) {
    const struct pkcs1_sig_prefix *sig_prefix = &kPKCS1SigPrefixes[i];
    if (sig_prefix->nid != hash_nid) {
      continue;
    }

    const uint8_t* prefix = sig_prefix->bytes;
    unsigned prefix_len = sig_prefix->len;
    unsigned signed_msg_len;
    uint8_t *signed_msg;

    signed_msg_len = prefix_len + msg_len;
    if (signed_msg_len < prefix_len) {
      OPENSSL_PUT_ERROR(RSA, RSA_R_TOO_LONG);
      return 0;
    }

    signed_msg = OPENSSL_malloc(signed_msg_len);
    if (!signed_msg) {
      OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
      return 0;
    }

    memcpy(signed_msg, prefix, prefix_len);
    memcpy(signed_msg + prefix_len, msg, msg_len);

    *out_msg = signed_msg;
    *out_msg_len = signed_msg_len;

    return 1;
  }

  OPENSSL_PUT_ERROR(RSA, RSA_R_UNKNOWN_ALGORITHM_TYPE);
  return 0;
}

int RSA_sign(int hash_nid, const uint8_t *in, unsigned in_len, uint8_t *out,
             unsigned *out_len, RSA *rsa, BN_BLINDING *blinding, RAND *rng) {
  const unsigned rsa_size = RSA_size(rsa);
  int ret = 0;
  uint8_t *signed_msg;
  size_t signed_msg_len;
  size_t size_t_out_len;

  if (!RSA_add_pkcs1_prefix(&signed_msg, &signed_msg_len, hash_nid, in,
                            in_len)) {
    return 0;
  }

  if (rsa_size < RSA_PKCS1_PADDING_SIZE ||
      signed_msg_len > rsa_size - RSA_PKCS1_PADDING_SIZE) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY);
    goto finish;
  }

  if (RSA_sign_raw(rsa, &size_t_out_len, out, rsa_size, signed_msg,
                   signed_msg_len, RSA_PKCS1_PADDING, blinding, rng)) {
    *out_len = size_t_out_len;
    ret = 1;
  }

finish:
  OPENSSL_free(signed_msg);
  return ret;
}

int RSA_check_key(const RSA *key, BN_CTX *ctx) {
  assert(ctx);

  BIGNUM n, pm1, qm1, lcm, gcd, de, dmp1, dmq1, iqmp;
  int ok = 0;

  BN_init(&n);
  BN_init(&pm1);
  BN_init(&qm1);
  BN_init(&lcm);
  BN_init(&gcd);
  BN_init(&de);
  BN_init(&dmp1);
  BN_init(&dmq1);
  BN_init(&iqmp);

  /* Technically |p < q| may be legal, but the implementation of |mod_exp| has
   * been optimized such that it is now required that |p > q|. |p == q| is
   * definitely *not* OK. To support keys with |p < q| in the future, we can
   * provide a function that swaps |p| and |q| and recalculates the CRT
   * parameters via the currently-deleted |RSA_recover_crt_params|. Or we can
   * just avoid using the CRT when |p < q|. */
  if (BN_cmp(key->p, key->q) <= 0) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_BAD_RSA_PARAMETERS);
    goto out;
  }

  if (/* n = pq */
      !BN_mul(&n, key->p, key->q, ctx) ||
      /* lcm = lcm(p-1, q-1) */
      !BN_sub(&pm1, key->p, BN_value_one()) ||
      !BN_sub(&qm1, key->q, BN_value_one()) ||
      !BN_mul(&lcm, &pm1, &qm1, ctx) ||
      !BN_gcd(&gcd, &pm1, &qm1, ctx) ||
      !BN_div(&lcm, NULL, &lcm, &gcd, ctx) ||
      /* de = d*e mod lcm(p-1, q-1) */
      !BN_mod_mul(&de, key->d, key->e, &lcm, ctx)) {
    OPENSSL_PUT_ERROR(RSA, ERR_LIB_BN);
    goto out;
  }

  if (BN_cmp(&n, key->n) != 0) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_N_NOT_EQUAL_P_Q);
    goto out;
  }

  if (!BN_is_one(&de)) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_D_E_NOT_CONGRUENT_TO_1);
    goto out;
  }

  if (/* dmp1 = d mod (p-1) */
      !BN_mod(&dmp1, key->d, &pm1, ctx) ||
      /* dmq1 = d mod (q-1) */
      !BN_mod(&dmq1, key->d, &qm1, ctx) ||
      /* iqmp = q^-1 mod p */
      !BN_mod_inverse(&iqmp, key->q, key->p, ctx)) {
    OPENSSL_PUT_ERROR(RSA, ERR_LIB_BN);
    goto out;
  }

  if (BN_cmp(&dmp1, key->dmp1) != 0 ||
      BN_cmp(&dmq1, key->dmq1) != 0 ||
      BN_cmp(&iqmp, key->iqmp) != 0) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_CRT_VALUES_INCORRECT);
    goto out;
  }

  ok = 1;

out:
  BN_free(&n);
  BN_free(&pm1);
  BN_free(&qm1);
  BN_free(&lcm);
  BN_free(&gcd);
  BN_free(&de);
  BN_free(&dmp1);
  BN_free(&dmq1);
  BN_free(&iqmp);

  return ok;
}
