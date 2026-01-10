#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

/*************************************************
 * Constant-Time Utility Functions
 *
 * These functions are designed to prevent timing side-channel attacks.
 * They must execute in constant time regardless of input values.
 *************************************************/

/**
 * Constant-time selection between two byte arrays.
 * If cond is 1, r = b. If cond is 0, r = a.
 * Assumes a, b, and r have length len.
 */
void select_bytes(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len,
                  uint8_t cond);

/**
 * Constant-time comparison of two byte arrays.
 * Returns 0 if equal, non-zero otherwise.
 */
int ct_memcmp(const uint8_t *a, const uint8_t *b, size_t len);

/**
 * Constant-time conditional move.
 * Copy len bytes from x to r if b is 1.
 */
void ct_cmov(uint8_t *r, const uint8_t *x, size_t len, uint8_t b);

#endif /* UTILS_H */
