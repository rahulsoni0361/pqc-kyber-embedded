/*************************************************
 * Polynomial Operations for Kyber
 *************************************************/

#include "../include/poly.h"
#include "../include/fips202.h"
#include "../include/ntt.h"
#include "../include/params.h"
#include <stdint.h>
#include <string.h>

/*************************************************
 * Name:        poly_zero
 *
 * Description: Initialize polynomial to zero
 *
 * Arguments:   - poly *p: pointer to polynomial
 *************************************************/
void poly_zero(poly *p) { memset(p->coeffs, 0, sizeof(p->coeffs)); }

/*************************************************
 * Name:        poly_add
 *
 * Description: Add two polynomials: r = a + b
 *
 * Arguments:   - poly *r: pointer to output polynomial
 *              - const poly *a: pointer to first input polynomial
 *              - const poly *b: pointer to second input polynomial
 *************************************************/
void poly_add(poly *r, const poly *a, const poly *b) {
  unsigned int i;
  for (i = 0; i < KYBER_N; i++)
    r->coeffs[i] = a->coeffs[i] + b->coeffs[i];
}

/*************************************************
 * Name:        poly_sub
 *
 * Description: Subtract two polynomials: r = a - b
 *
 * Arguments:   - poly *r: pointer to output polynomial
 *              - const poly *a: pointer to first input polynomial
 *              - const poly *b: pointer to second input polynomial
 *************************************************/
void poly_sub(poly *r, const poly *a, const poly *b) {
  unsigned int i;
  for (i = 0; i < KYBER_N; i++)
    r->coeffs[i] = a->coeffs[i] - b->coeffs[i];
}

/*************************************************
 * Name:        poly_reduce
 *
 * Description: Apply Barrett reduction to all coefficients of a polynomial
 *              to reduce them to the standard representative
 *
 * Arguments:   - poly *r: pointer to polynomial to be reduced
 *************************************************/
void poly_reduce(poly *r) {
  unsigned int i;
  for (i = 0; i < KYBER_N; i++)
    r->coeffs[i] = barrett_reduce(r->coeffs[i]);
}

/*************************************************
 * Name:        poly_ntt
 *
 * Description: Apply NTT to polynomial
 *
 * Arguments:   - poly *r: pointer to polynomial
 *************************************************/
void poly_ntt(poly *r) {
  ntt(r->coeffs);
  poly_reduce(r);
}

/*************************************************
 * Name:        poly_invntt
 *
 * Description: Apply inverse NTT to polynomial
 *
 * Arguments:   - poly *r: pointer to polynomial
 *************************************************/
void poly_invntt(poly *r) { invntt(r->coeffs); }

/*************************************************
 * Name:        poly_basemul_montgomery
 *
 * Description: Multiplication of two polynomials in NTT domain
 *
 * Arguments:   - poly *r: pointer to output polynomial
 *              - const poly *a: pointer to first input polynomial
 *              - const poly *b: pointer to second input polynomial
 *************************************************/
void poly_basemul_montgomery(poly *r, const poly *a, const poly *b) {
  unsigned int i;
  for (i = 0; i < KYBER_N / 4; i++) {
    basemul(&r->coeffs[4 * i], &a->coeffs[4 * i], &b->coeffs[4 * i],
            zetas[64 + i]);
    basemul(&r->coeffs[4 * i + 2], &a->coeffs[4 * i + 2], &b->coeffs[4 * i + 2],
            -zetas[64 + i]);
  }
}

/*************************************************
 * Name:        poly_tomont
 *
 * Description: Convert polynomial to Montgomery domain
 *
 * Arguments:   - poly *r: pointer to polynomial
 *************************************************/
void poly_tomont(poly *r) {
  unsigned int i;
  const int16_t f = (1ULL << 32) % KYBER_Q;
  for (i = 0; i < KYBER_N; i++)
    r->coeffs[i] = montgomery_reduce((int32_t)r->coeffs[i] * f);
}

/*************************************************
 * Name:        poly_tobytes
 *
 * Description: Serialization of a polynomial
 *
 * Arguments:   - uint8_t *r: pointer to output byte array (needs
 * KYBER_POLYBYTES bytes)
 *              - const poly *a: pointer to input polynomial
 *************************************************/
void poly_tobytes(uint8_t *r, const poly *a) {
  unsigned int i;
  uint16_t t0, t1;

  for (i = 0; i < KYBER_N / 2; i++) {
    // Map to positive standard representatives
    t0 = a->coeffs[2 * i];
    t0 += ((int16_t)t0 >> 15) & KYBER_Q;
    t1 = a->coeffs[2 * i + 1];
    t1 += ((int16_t)t1 >> 15) & KYBER_Q;
    r[3 * i + 0] = (t0 >> 0);
    r[3 * i + 1] = (t0 >> 8) | (t1 << 4);
    r[3 * i + 2] = (t1 >> 4);
  }
}

/*************************************************
 * Name:        poly_frombytes
 *
 * Description: De-serialization of a polynomial
 *
 * Arguments:   - poly *r: pointer to output polynomial
 *              - const uint8_t *a: pointer to input byte array
 *************************************************/
void poly_frombytes(poly *r, const uint8_t *a) {
  unsigned int i;
  for (i = 0; i < KYBER_N / 2; i++) {
    r->coeffs[2 * i] =
        ((a[3 * i + 0] >> 0) | ((uint16_t)a[3 * i + 1] << 8)) & 0xFFF;
    r->coeffs[2 * i + 1] =
        ((a[3 * i + 1] >> 4) | ((uint16_t)a[3 * i + 2] << 4)) & 0xFFF;
  }
}

/*************************************************
 * Name:        poly_compress
 *
 * Description: Compression of polynomial coefficients
 *
 * Arguments:   - uint8_t *r: pointer to output byte array
 *              - const poly *a: pointer to input polynomial
 *              - int d: number of bits per coefficient
 *************************************************/
void poly_compress(uint8_t *r, const poly *a, int d) {
  unsigned int i, j;
  int16_t u;
  uint8_t t[8];

  if (d == 4) {
    for (i = 0; i < KYBER_N / 2; i++) {
      for (j = 0; j < 2; j++) {
        u = a->coeffs[2 * i + j];
        u += ((int16_t)u >> 15) & KYBER_Q;
        t[j] = ((((uint16_t)u << 4) + KYBER_Q / 2) / KYBER_Q) & 15;
      }
      r[i] = t[0] | (t[1] << 4);
    }
  } else if (d == 5) {
    for (i = 0; i < KYBER_N / 8; i++) {
      for (j = 0; j < 8; j++) {
        u = a->coeffs[8 * i + j];
        u += ((int16_t)u >> 15) & KYBER_Q;
        t[j] = ((((uint32_t)u << 5) + KYBER_Q / 2) / KYBER_Q) & 31;
      }
      r[5 * i + 0] = (t[0] >> 0) | (t[1] << 5);
      r[5 * i + 1] = (t[1] >> 3) | (t[2] << 2) | (t[3] << 7);
      r[5 * i + 2] = (t[3] >> 1) | (t[4] << 4);
      r[5 * i + 3] = (t[4] >> 4) | (t[5] << 1) | (t[6] << 6);
      r[5 * i + 4] = (t[6] >> 2) | (t[7] << 3);
    }
  }
}

/*************************************************
 * Name:        poly_decompress
 *
 * Description: Decompression of polynomial coefficients
 *
 * Arguments:   - poly *r: pointer to output polynomial
 *              - const uint8_t *a: pointer to input byte array
 *              - int d: number of bits per coefficient
 *************************************************/
void poly_decompress(poly *r, const uint8_t *a, int d) {
  unsigned int i;

  if (d == 4) {
    for (i = 0; i < KYBER_N / 2; i++) {
      r->coeffs[2 * i + 0] = (((uint16_t)(a[i] & 15) * KYBER_Q) + 8) >> 4;
      r->coeffs[2 * i + 1] = (((uint16_t)(a[i] >> 4) * KYBER_Q) + 8) >> 4;
    }
  } else if (d == 5) {
    unsigned int j;
    uint8_t t[8];
    for (i = 0; i < KYBER_N / 8; i++) {
      t[0] = (a[5 * i + 0] >> 0);
      t[1] = (a[5 * i + 0] >> 5) | (a[5 * i + 1] << 3);
      t[2] = (a[5 * i + 1] >> 2);
      t[3] = (a[5 * i + 1] >> 7) | (a[5 * i + 2] << 1);
      t[4] = (a[5 * i + 2] >> 4) | (a[5 * i + 3] << 4);
      t[5] = (a[5 * i + 3] >> 1);
      t[6] = (a[5 * i + 3] >> 6) | (a[5 * i + 4] << 2);
      t[7] = (a[5 * i + 4] >> 3);
      for (j = 0; j < 8; j++)
        r->coeffs[8 * i + j] = (((uint32_t)(t[j] & 31) * KYBER_Q) + 16) >> 5;
    }
  }
}

/*************************************************
 * Centered Binomial Distribution (CBD) sampling
 *************************************************/

// Helper to load 3 bytes as uint32
static uint32_t load24_littleendian(const uint8_t x[3]) {
  uint32_t r;
  r = (uint32_t)x[0];
  r |= (uint32_t)x[1] << 8;
  r |= (uint32_t)x[2] << 16;
  return r;
}

// Helper to load 4 bytes as uint32
static uint32_t load32_littleendian(const uint8_t x[4]) {
  uint32_t r;
  r = (uint32_t)x[0];
  r |= (uint32_t)x[1] << 8;
  r |= (uint32_t)x[2] << 16;
  r |= (uint32_t)x[3] << 24;
  return r;
}

/*************************************************
 * Name:        cbd2
 *
 * Description: Sample polynomial from CBD with eta=2
 *************************************************/
static void cbd2(poly *r, const uint8_t buf[2 * KYBER_N / 4]) {
  unsigned int i, j;
  uint32_t t, d;
  int16_t a, b;

  for (i = 0; i < KYBER_N / 8; i++) {
    t = load32_littleendian(buf + 4 * i);
    d = t & 0x55555555;
    d += (t >> 1) & 0x55555555;

    for (j = 0; j < 8; j++) {
      a = (d >> (4 * j + 0)) & 0x3;
      b = (d >> (4 * j + 2)) & 0x3;
      r->coeffs[8 * i + j] = a - b;
    }
  }
}

/*************************************************
 * Name:        cbd3
 *
 * Description: Sample polynomial from CBD with eta=3
 *************************************************/
static void cbd3(poly *r, const uint8_t buf[3 * KYBER_N / 4]) {
  unsigned int i, j;
  uint32_t t, d;
  int16_t a, b;

  for (i = 0; i < KYBER_N / 4; i++) {
    t = load24_littleendian(buf + 3 * i);
    d = t & 0x00249249;
    d += (t >> 1) & 0x00249249;
    d += (t >> 2) & 0x00249249;

    for (j = 0; j < 4; j++) {
      a = (d >> (6 * j + 0)) & 0x7;
      b = (d >> (6 * j + 3)) & 0x7;
      r->coeffs[4 * i + j] = a - b;
    }
  }
}

/*************************************************
 * Name:        poly_cbd_eta1
 *
 * Description: Sample polynomial from CBD with eta1
 *************************************************/
void poly_cbd_eta1(poly *r, const uint8_t *buf) {
#if KYBER_ETA1 == 2
  cbd2(r, buf);
#elif KYBER_ETA1 == 3
  cbd3(r, buf);
#else
#error "Invalid KYBER_ETA1"
#endif
}

/*************************************************
 * Name:        poly_cbd_eta2
 *
 * Description: Sample polynomial from CBD with eta2
 *************************************************/
void poly_cbd_eta2(poly *r, const uint8_t *buf) {
#if KYBER_ETA2 == 2
  cbd2(r, buf);
#else
#error "Invalid KYBER_ETA2"
#endif
}

/*************************************************
 * Name:        poly_getnoise_eta1
 *
 * Description: Sample a polynomial deterministically from a seed
 *              and nonce using SHAKE256 and CBD with eta1
 *************************************************/
void poly_getnoise_eta1(poly *r, const uint8_t seed[KYBER_SYMBYTES],
                        uint8_t nonce) {
  uint8_t buf[KYBER_ETA1 * KYBER_N / 4];
  uint8_t extkey[KYBER_SYMBYTES + 1];

  memcpy(extkey, seed, KYBER_SYMBYTES);
  extkey[KYBER_SYMBYTES] = nonce;

  shake256(buf, sizeof(buf), extkey, sizeof(extkey));
  poly_cbd_eta1(r, buf);
}

/*************************************************
 * Name:        poly_getnoise_eta2
 *
 * Description: Sample a polynomial deterministically from a seed
 *              and nonce using SHAKE256 and CBD with eta2
 *************************************************/
void poly_getnoise_eta2(poly *r, const uint8_t seed[KYBER_SYMBYTES],
                        uint8_t nonce) {
  uint8_t buf[KYBER_ETA2 * KYBER_N / 4];
  uint8_t extkey[KYBER_SYMBYTES + 1];

  memcpy(extkey, seed, KYBER_SYMBYTES);
  extkey[KYBER_SYMBYTES] = nonce;

  shake256(buf, sizeof(buf), extkey, sizeof(extkey));
  poly_cbd_eta2(r, buf);
}
