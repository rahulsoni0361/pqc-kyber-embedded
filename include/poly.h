#ifndef POLY_H
#define POLY_H

#include "params.h"
#include <stdint.h>

/*************************************************
 * Name:        poly
 *
 * Description: Structure representing a polynomial
 *              with KYBER_N coefficients in Z_q
 *************************************************/
typedef struct {
  int16_t coeffs[KYBER_N];
} poly;

/*************************************************
 * Polynomial Operations
 *************************************************/

// Initialize polynomial to zero
void poly_zero(poly *p);

// Add two polynomials: r = a + b
void poly_add(poly *r, const poly *a, const poly *b);

// Subtract two polynomials: r = a - b
void poly_sub(poly *r, const poly *a, const poly *b);

// Compress polynomial coefficients
void poly_compress(uint8_t *r, const poly *a, int d);

// Decompress polynomial coefficients
void poly_decompress(poly *r, const uint8_t *a, int d);

// Encode polynomial to bytes
void poly_tobytes(uint8_t *r, const poly *a);

// Decode polynomial from bytes
void poly_frombytes(poly *r, const uint8_t *a);

// Sample polynomial from centered binomial distribution
void poly_cbd_eta1(poly *r, const uint8_t *buf);
void poly_cbd_eta2(poly *r, const uint8_t *buf);

// NTT and inverse NTT
void poly_ntt(poly *r);
void poly_invntt(poly *r);

// Pointwise multiplication in NTT domain
void poly_basemul_montgomery(poly *r, const poly *a, const poly *b);

// Convert to Montgomery form
void poly_tomont(poly *r);

// Reduce coefficients mod q
void poly_reduce(poly *r);

// Sample polynomial from XOF (for matrix generation)
void poly_getnoise_eta1(poly *r, const uint8_t seed[KYBER_SYMBYTES],
                        uint8_t nonce);
void poly_getnoise_eta2(poly *r, const uint8_t seed[KYBER_SYMBYTES],
                        uint8_t nonce);

// Message encoding/decoding
void poly_frommsg(poly *r, const uint8_t msg[KYBER_SYMBYTES]);
void poly_tomsg(uint8_t msg[KYBER_SYMBYTES], const poly *a);

#endif /* POLY_H */
