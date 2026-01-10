/*************************************************
 * Polynomial Vector Operations for Kyber
 *************************************************/

#include "../include/polyvec.h"
#include "../include/params.h"
#include "../include/poly.h"
#include <stdint.h>

/*************************************************
 * Name:        polyvec_zero
 *
 * Description: Initialize all polynomials in vector to zero
 *************************************************/
void polyvec_zero(polyvec *v) {
  unsigned int i;
  for (i = 0; i < KYBER_K; i++)
    poly_zero(&v->vec[i]);
}

/*************************************************
 * Name:        polyvec_add
 *
 * Description: Add vectors: r = a + b
 *************************************************/
void polyvec_add(polyvec *r, const polyvec *a, const polyvec *b) {
  unsigned int i;
  for (i = 0; i < KYBER_K; i++)
    poly_add(&r->vec[i], &a->vec[i], &b->vec[i]);
}

/*************************************************
 * Name:        polyvec_ntt
 *
 * Description: Apply NTT to all polynomials in vector
 *************************************************/
void polyvec_ntt(polyvec *r) {
  unsigned int i;
  for (i = 0; i < KYBER_K; i++)
    poly_ntt(&r->vec[i]);
}

/*************************************************
 * Name:        polyvec_invntt
 *
 * Description: Apply inverse NTT to all polynomials in vector
 *************************************************/
void polyvec_invntt(polyvec *r) {
  unsigned int i;
  for (i = 0; i < KYBER_K; i++)
    poly_invntt(&r->vec[i]);
}

/*************************************************
 * Name:        polyvec_reduce
 *
 * Description: Apply Barrett reduction to all coefficients in vector
 *************************************************/
void polyvec_reduce(polyvec *r) {
  unsigned int i;
  for (i = 0; i < KYBER_K; i++)
    poly_reduce(&r->vec[i]);
}

/*************************************************
 * Name:        polyvec_pointwise_acc_montgomery
 *
 * Description: Pointwise multiply elements of a and b, accumulate into r.
 *              This computes the inner product (dot product) of two vectors.
 *************************************************/
void polyvec_pointwise_acc_montgomery(poly *r, const polyvec *a,
                                      const polyvec *b) {
  unsigned int i;
  poly t;

  poly_basemul_montgomery(r, &a->vec[0], &b->vec[0]);
  for (i = 1; i < KYBER_K; i++) {
    poly_basemul_montgomery(&t, &a->vec[i], &b->vec[i]);
    poly_add(r, r, &t);
  }
  poly_reduce(r);
}

/*************************************************
 * Name:        polyvec_tobytes
 *
 * Description: Serialize vector of polynomials
 *************************************************/
void polyvec_tobytes(uint8_t *r, const polyvec *a) {
  unsigned int i;
  for (i = 0; i < KYBER_K; i++)
    poly_tobytes(r + i * 384, &a->vec[i]);
}

/*************************************************
 * Name:        polyvec_frombytes
 *
 * Description: De-serialize vector of polynomials
 *************************************************/
void polyvec_frombytes(polyvec *r, const uint8_t *a) {
  unsigned int i;
  for (i = 0; i < KYBER_K; i++)
    poly_frombytes(&r->vec[i], a + i * 384);
}

/*************************************************
 * Name:        polyvec_compress
 *
 * Description: Compress and serialize vector of polynomials
 *************************************************/
void polyvec_compress(uint8_t *r, const polyvec *a) {
  unsigned int i, j, k;
  uint16_t t[4];

#if (KYBER_DU == 10)
  for (i = 0; i < KYBER_K; i++) {
    for (j = 0; j < KYBER_N / 4; j++) {
      for (k = 0; k < 4; k++) {
        t[k] = a->vec[i].coeffs[4 * j + k];
        t[k] += ((int16_t)t[k] >> 15) & KYBER_Q;
        t[k] = ((((uint32_t)t[k] << 10) + KYBER_Q / 2) / KYBER_Q) & 0x3ff;
      }
      r[0] = (t[0] >> 0);
      r[1] = (t[0] >> 8) | (t[1] << 2);
      r[2] = (t[1] >> 6) | (t[2] << 4);
      r[3] = (t[2] >> 4) | (t[3] << 6);
      r[4] = (t[3] >> 2);
      r += 5;
    }
  }
#elif (KYBER_DU == 11)
  for (i = 0; i < KYBER_K; i++) {
    for (j = 0; j < KYBER_N / 8; j++) {
      for (k = 0; k < 8; k++) {
        t[k] = a->vec[i].coeffs[8 * j + k];
        t[k] += ((int16_t)t[k] >> 15) & KYBER_Q;
        t[k] = ((((uint32_t)t[k] << 11) + KYBER_Q / 2) / KYBER_Q) & 0x7ff;
      }
      r[0] = (t[0] >> 0);
      r[1] = (t[0] >> 8) | (t[1] << 3);
      r[2] = (t[1] >> 5) | (t[2] << 6);
      r[3] = (t[2] >> 2);
      r[4] = (t[2] >> 10) | (t[3] << 1);
      r[5] = (t[3] >> 7) | (t[4] << 4);
      r[6] = (t[4] >> 4) | (t[5] << 7);
      r[7] = (t[5] >> 1);
      r[8] = (t[5] >> 9) | (t[6] << 2);
      r[9] = (t[6] >> 6) | (t[7] << 5);
      r[10] = (t[7] >> 3);
      r += 11;
    }
  }
#endif
}

/*************************************************
 * Name:        polyvec_decompress
 *
 * Description: De-serialize and decompress vector of polynomials
 *************************************************/
void polyvec_decompress(polyvec *r, const uint8_t *a) {
  unsigned int i, j, k;
  uint16_t t[8];

#if (KYBER_DU == 10)
  for (i = 0; i < KYBER_K; i++) {
    for (j = 0; j < KYBER_N / 4; j++) {
      t[0] = (a[0] >> 0) | ((uint16_t)a[1] << 8);
      t[1] = (a[1] >> 2) | ((uint16_t)a[2] << 6);
      t[2] = (a[2] >> 4) | ((uint16_t)a[3] << 4);
      t[3] = (a[3] >> 6) | ((uint16_t)a[4] << 2);
      a += 5;
      for (k = 0; k < 4; k++)
        r->vec[i].coeffs[4 * j + k] =
            ((uint32_t)(t[k] & 0x3FF) * KYBER_Q + 512) >> 10;
    }
  }
#elif (KYBER_DU == 11)
  for (i = 0; i < KYBER_K; i++) {
    for (j = 0; j < KYBER_N / 8; j++) {
      t[0] = (a[0] >> 0) | ((uint16_t)a[1] << 8);
      t[1] = (a[1] >> 3) | ((uint16_t)a[2] << 5);
      t[2] = (a[2] >> 6) | ((uint16_t)a[3] << 2) | ((uint16_t)a[4] << 10);
      t[3] = (a[4] >> 1) | ((uint16_t)a[5] << 7);
      t[4] = (a[5] >> 4) | ((uint16_t)a[6] << 4);
      t[5] = (a[6] >> 7) | ((uint16_t)a[7] << 1) | ((uint16_t)a[8] << 9);
      t[6] = (a[8] >> 2) | ((uint16_t)a[9] << 6);
      t[7] = (a[9] >> 5) | ((uint16_t)a[10] << 3);
      a += 11;
      for (k = 0; k < 8; k++)
        r->vec[i].coeffs[8 * j + k] =
            ((uint32_t)(t[k] & 0x7FF) * KYBER_Q + 1024) >> 11;
    }
  }
#endif
}
