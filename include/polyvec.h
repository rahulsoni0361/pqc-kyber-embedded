#ifndef POLYVEC_H
#define POLYVEC_H

#include "params.h"
#include "poly.h"
#include <stdint.h>


/*************************************************
 * Name:        polyvec
 *
 * Description: Structure representing a vector of
 *              KYBER_K polynomials (module element)
 *************************************************/
typedef struct {
  poly vec[KYBER_K];
} polyvec;

/*************************************************
 * Polynomial Vector Operations
 *************************************************/

// Initialize vector to zero
void polyvec_zero(polyvec *v);

// Add two vectors: r = a + b
void polyvec_add(polyvec *r, const polyvec *a, const polyvec *b);

// Compress vector
void polyvec_compress(uint8_t *r, const polyvec *a);

// Decompress vector
void polyvec_decompress(polyvec *r, const uint8_t *a);

// Encode vector to bytes
void polyvec_tobytes(uint8_t *r, const polyvec *a);

// Decode vector from bytes
void polyvec_frombytes(polyvec *r, const uint8_t *a);

// Apply NTT to all polynomials in vector
void polyvec_ntt(polyvec *r);

// Apply inverse NTT to all polynomials in vector
void polyvec_invntt(polyvec *r);

// Pointwise multiplication and accumulation: r = a · b (dot product in NTT
// domain)
void polyvec_pointwise_acc_montgomery(poly *r, const polyvec *a,
                                      const polyvec *b);

// Reduce all coefficients in vector
void polyvec_reduce(polyvec *r);

#endif /* POLYVEC_H */
