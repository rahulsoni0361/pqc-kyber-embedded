/*************************************************
 * IND-CPA secure Public Key Encryption for Kyber
 *
 * Implements Algorithms 4, 5, 6 from the Kyber specification
 *************************************************/

#include "../include/indcpa.h"
#include "../include/fips202.h"
#include "../include/ntt.h"
#include "../include/params.h"
#include "../include/poly.h"
#include "../include/polyvec.h"
#include <stdint.h>
#include <string.h>

/*************************************************
 * Name:        pack_pk
 *
 * Description: Serialize the public key as concatenation of
 *              the serialized vector of polynomials pk
 *              and the public seed used to generate the matrix A.
 *************************************************/
static void pack_pk(uint8_t r[KYBER_PUBLICKEYBYTES], const polyvec *pk,
                    const uint8_t seed[KYBER_SYMBYTES]) {
  polyvec_tobytes(r, pk);
  memcpy(r + KYBER_POLYVECBYTES, seed, KYBER_SYMBYTES);
}

/*************************************************
 * Name:        unpack_pk
 *
 * Description: De-serialize public key from a byte array
 *************************************************/
static void unpack_pk(polyvec *pk, uint8_t seed[KYBER_SYMBYTES],
                      const uint8_t packedpk[KYBER_PUBLICKEYBYTES]) {
  polyvec_frombytes(pk, packedpk);
  memcpy(seed, packedpk + KYBER_POLYVECBYTES, KYBER_SYMBYTES);
}

/*************************************************
 * Name:        pack_sk
 *
 * Description: Serialize the secret key
 *************************************************/
static void pack_sk(uint8_t r[KYBER_POLYVECBYTES], const polyvec *sk) {
  polyvec_tobytes(r, sk);
}

/*************************************************
 * Name:        unpack_sk
 *
 * Description: De-serialize the secret key
 *************************************************/
static void unpack_sk(polyvec *sk, const uint8_t packedsk[KYBER_POLYVECBYTES]) {
  polyvec_frombytes(sk, packedsk);
}

/*************************************************
 * Name:        pack_ciphertext
 *
 * Description: Serialize ciphertext
 *************************************************/
static void pack_ciphertext(uint8_t r[KYBER_CIPHERTEXTBYTES], const polyvec *b,
                            const poly *v) {
  polyvec_compress(r, b);
  poly_compress(r + KYBER_POLYVECCOMPRESSEDBYTES, v, KYBER_DV);
}

/*************************************************
 * Name:        unpack_ciphertext
 *
 * Description: De-serialize ciphertext
 *************************************************/
static void unpack_ciphertext(polyvec *b, poly *v,
                              const uint8_t c[KYBER_CIPHERTEXTBYTES]) {
  polyvec_decompress(b, c);
  poly_decompress(v, c + KYBER_POLYVECCOMPRESSEDBYTES, KYBER_DV);
}

/*************************************************
 * Name:        rej_uniform
 *
 * Description: Sample uniformly random elements in [0, q)
 *              using rejection sampling
 *************************************************/
static unsigned int rej_uniform(int16_t *r, unsigned int len,
                                const uint8_t *buf, unsigned int buflen) {
  unsigned int ctr, pos;
  uint16_t val0, val1;

  ctr = pos = 0;
  while (ctr < len && pos + 3 <= buflen) {
    val0 = ((buf[pos + 0] >> 0) | ((uint16_t)buf[pos + 1] << 8)) & 0xFFF;
    val1 = ((buf[pos + 1] >> 4) | ((uint16_t)buf[pos + 2] << 4)) & 0xFFF;
    pos += 3;

    if (val0 < KYBER_Q)
      r[ctr++] = val0;
    if (ctr < len && val1 < KYBER_Q)
      r[ctr++] = val1;
  }

  return ctr;
}

#define GEN_A_NBLOCKS                                                          \
  ((12 * KYBER_N / 8 * (1 << 12) / KYBER_Q + SHAKE128_RATE) / SHAKE128_RATE)

/*************************************************
 * Name:        gen_matrix
 *
 * Description: Deterministically generate matrix A (or transposed)
 *              from a seed. Entries are polynomials that look uniformly random.
 *************************************************/
static void gen_matrix(polyvec *a, const uint8_t seed[KYBER_SYMBYTES],
                       int transposed) {
  unsigned int ctr, i, j, k;
  unsigned int buflen, off;
  uint8_t buf[GEN_A_NBLOCKS * SHAKE128_RATE + 2];
  keccak_state state;

  for (i = 0; i < KYBER_K; i++) {
    for (j = 0; j < KYBER_K; j++) {
      if (transposed) {
        buf[0] = i;
        buf[1] = j;
      } else {
        buf[0] = j;
        buf[1] = i;
      }

      // Absorb seed || indices
      uint8_t extseed[KYBER_SYMBYTES + 2];
      memcpy(extseed, seed, KYBER_SYMBYTES);
      extseed[KYBER_SYMBYTES] = buf[0];
      extseed[KYBER_SYMBYTES + 1] = buf[1];

      shake128(buf, sizeof(buf), extseed, sizeof(extseed));

      buflen = GEN_A_NBLOCKS * SHAKE128_RATE;
      ctr = rej_uniform(a[i].vec[j].coeffs, KYBER_N, buf, buflen);

      while (ctr < KYBER_N) {
        off = buflen % 3;
        for (k = 0; k < off; k++)
          buf[k] = buf[buflen - off + k];
        shake128(buf + off, SHAKE128_RATE, extseed, sizeof(extseed));
        buflen = off + SHAKE128_RATE;
        ctr +=
            rej_uniform(a[i].vec[j].coeffs + ctr, KYBER_N - ctr, buf, buflen);
      }
    }
  }
}

/*************************************************
 * Name:        indcpa_keypair
 *
 * Description: Generates public and private key for IND-CPA encryption
 *
 * Algorithm 4 from Kyber spec
 *************************************************/
void indcpa_keypair(uint8_t pk[KYBER_PUBLICKEYBYTES],
                    uint8_t sk[KYBER_SECRETKEYBYTES]) {
  unsigned int i;
  uint8_t buf[2 * KYBER_SYMBYTES];
  const uint8_t *publicseed = buf;
  const uint8_t *noiseseed = buf + KYBER_SYMBYTES;
  polyvec a[KYBER_K], e, pkpv, skpv;
  uint8_t nonce = 0;

  // Generate random bytes
  // In production, use a secure RNG. For now, we'll use a placeholder.
  // randombytes(buf, KYBER_SYMBYTES);
  memset(buf, 0x42, KYBER_SYMBYTES); // Placeholder - replace with real RNG

  // Hash to get public seed and noise seed
  sha3_512(buf, buf, KYBER_SYMBYTES);

  // Generate matrix A
  gen_matrix(a, publicseed, 0);

  // Sample secret vector s
  for (i = 0; i < KYBER_K; i++)
    poly_getnoise_eta1(&skpv.vec[i], noiseseed, nonce++);

  // Sample error vector e
  for (i = 0; i < KYBER_K; i++)
    poly_getnoise_eta1(&e.vec[i], noiseseed, nonce++);

  // Convert s to NTT domain
  polyvec_ntt(&skpv);
  polyvec_ntt(&e);

  // Compute t = As + e
  for (i = 0; i < KYBER_K; i++) {
    polyvec_pointwise_acc_montgomery(&pkpv.vec[i], &a[i], &skpv);
    poly_tomont(&pkpv.vec[i]);
  }

  polyvec_add(&pkpv, &pkpv, &e);
  polyvec_reduce(&pkpv);

  // Pack keys
  pack_sk(sk, &skpv);
  pack_pk(pk, &pkpv, publicseed);
}

/*************************************************
 * Name:        indcpa_enc
 *
 * Description: Encryption function
 *
 * Algorithm 5 from Kyber spec
 *************************************************/
void indcpa_enc(uint8_t c[KYBER_CIPHERTEXTBYTES],
                const uint8_t m[KYBER_SYMBYTES],
                const uint8_t pk[KYBER_PUBLICKEYBYTES],
                const uint8_t coins[KYBER_SYMBYTES]) {
  unsigned int i;
  uint8_t seed[KYBER_SYMBYTES];
  uint8_t nonce = 0;
  polyvec sp, pkpv, ep, at[KYBER_K], b;
  poly v, k, epp;

  // Unpack public key
  unpack_pk(&pkpv, seed, pk);

  // Encode message as polynomial
  poly_frommsg(&k, m);

  // Generate transposed matrix A^T
  gen_matrix(at, seed, 1);

  // Sample secret vector r (sp)
  for (i = 0; i < KYBER_K; i++)
    poly_getnoise_eta1(&sp.vec[i], coins, nonce++);

  // Sample error vector e1 (ep)
  for (i = 0; i < KYBER_K; i++)
    poly_getnoise_eta2(&ep.vec[i], coins, nonce++);

  // Sample error polynomial e2 (epp)
  poly_getnoise_eta2(&epp, coins, nonce++);

  // NTT(r)
  polyvec_ntt(&sp);

  // Compute u = A^T * r + e1
  for (i = 0; i < KYBER_K; i++)
    polyvec_pointwise_acc_montgomery(&b.vec[i], &at[i], &sp);

  polyvec_invntt(&b);
  polyvec_add(&b, &b, &ep);
  polyvec_reduce(&b);

  // Compute v = t^T * r + e2 + m
  polyvec_pointwise_acc_montgomery(&v, &pkpv, &sp);
  poly_invntt(&v);
  poly_add(&v, &v, &epp);
  poly_add(&v, &v, &k);
  poly_reduce(&v);

  // Pack ciphertext
  pack_ciphertext(c, &b, &v);
}

/*************************************************
 * Name:        indcpa_dec
 *
 * Description: Decryption function
 *
 * Algorithm 6 from Kyber spec
 *************************************************/
void indcpa_dec(uint8_t m[KYBER_SYMBYTES],
                const uint8_t c[KYBER_CIPHERTEXTBYTES],
                const uint8_t sk[KYBER_SECRETKEYBYTES]) {
  polyvec b, skpv;
  poly v, mp;

  // Unpack ciphertext and secret key
  unpack_ciphertext(&b, &v, c);
  unpack_sk(&skpv, sk);

  // NTT(u)
  polyvec_ntt(&b);

  // Compute m = v - s^T * u
  polyvec_pointwise_acc_montgomery(&mp, &skpv, &b);
  poly_invntt(&mp);

  poly_sub(&mp, &v, &mp);
  poly_reduce(&mp);

  // Decode message
  poly_tomsg(m, &mp);
}

/*************************************************
 * Helper functions for message encoding
 *************************************************/
void poly_frommsg(poly *r, const uint8_t msg[KYBER_SYMBYTES]) {
  unsigned int i, j;
  int16_t mask;

  for (i = 0; i < KYBER_N / 8; i++) {
    for (j = 0; j < 8; j++) {
      mask = -(int16_t)((msg[i] >> j) & 1);
      r->coeffs[8 * i + j] = mask & ((KYBER_Q + 1) / 2);
    }
  }
}

void poly_tomsg(uint8_t msg[KYBER_SYMBYTES], const poly *a) {
  unsigned int i, j;
  uint16_t t;

  for (i = 0; i < KYBER_N / 8; i++) {
    msg[i] = 0;
    for (j = 0; j < 8; j++) {
      t = a->coeffs[8 * i + j];
      t += ((int16_t)t >> 15) & KYBER_Q;
      t = (((t << 1) + KYBER_Q / 2) / KYBER_Q) & 1;
      msg[i] |= t << j;
    }
  }
}
