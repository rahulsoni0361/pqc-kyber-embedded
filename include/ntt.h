#ifndef NTT_H
#define NTT_H

#include "params.h"
#include <stdint.h>


/*************************************************
 * NTT Constants
 *
 * Kyber uses a 256-point NTT with:
 * - Prime q = 3329
 * - Primitive 256th root of unity: zeta = 17
 * - Montgomery constant R = 2^16 mod q
 *************************************************/

#define MONT 2285  // 2^16 mod q
#define QINV 62209 // q^(-1) mod 2^16

/*************************************************
 * Name:        ntt
 *
 * Description: Inplace number-theoretic transform (NTT) in Rq.
 *              input is in standard order, output is in bit-reversed order
 *
 * Arguments:   - int16_t r[256]: pointer to input/output vector of elements of
 * Zq
 **************************************************/
void ntt(int16_t r[KYBER_N]);

/*************************************************
 * Name:        invntt
 *
 * Description: Inplace inverse number-theoretic transform in Rq and
 *              multiplication by Montgomery factor 2^16.
 *              Input is in bit-reversed order, output is in standard order
 *
 * Arguments:   - int16_t r[256]: pointer to input/output vector of elements of
 * Zq
 **************************************************/
void invntt(int16_t r[KYBER_N]);

/*************************************************
 * Name:        basemul
 *
 * Description: Multiplication of polynomials in Zq[X]/(X^2-zeta)
 *              used for multiplication in Zq[X]/(X^256+1) in NTT domain
 *
 * Arguments:   - int16_t r[2]: pointer to 2 output coefficients
 *              - const int16_t a[2]: pointer to 2 input coefficients
 *              - const int16_t b[2]: pointer to 2 input coefficients
 *              - int16_t zeta: root of unity for the specific base case
 **************************************************/
void basemul(int16_t r[2], const int16_t a[2], const int16_t b[2],
             int16_t zeta);

/*************************************************
 * Name:        montgomery_reduce
 *
 * Description: Montgomery reduction; given a 32-bit integer a, computes
 *              16-bit integer congruent to a * R^-1 mod q, where R=2^16
 *
 * Arguments:   - int32_t a: input integer to be reduced
 *
 * Returns:     integer in {-q+1,...,q-1} congruent to a * R^-1 modulo q.
 **************************************************/
int16_t montgomery_reduce(int32_t a);

/*************************************************
 * Name:        barrett_reduce
 *
 * Description: Barrett reduction; given a 16-bit integer a, computes
 *              centered representative congruent to a mod q in
 * {-(q-1)/2,...,(q-1)/2}
 *
 * Arguments:   - int16_t a: input integer to be reduced
 *
 * Returns:     integer in {-(q-1)/2,...,(q-1)/2} congruent to a modulo q.
 **************************************************/
int16_t barrett_reduce(int16_t a);

/*************************************************
 * Precomputed NTT twiddle factors (zetas)
 * These are powers of the primitive 256th root of unity
 * in Montgomery form and bit-reversed order
 *************************************************/
extern const int16_t zetas[128];

#endif /* NTT_H */
