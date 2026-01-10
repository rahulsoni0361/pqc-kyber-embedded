/*************************************************
 * Utility Functions for Kyber
 *************************************************/

#include "../include/utils.h"
#include <stdint.h>

/*************************************************
 * Name:        select_bytes
 *
 * Description: Constant-time conditional selection of byte arrays.
 *              If cond is 1, copies b to r. If cond is 0, copies a to r.
 *              CRITICAL: This must be constant-time to prevent timing attacks.
 *
 * Arguments:   - uint8_t *r: output buffer
 *              - const uint8_t *a: first input (selected if cond == 0)
 *              - const uint8_t *b: second input (selected if cond == 1)
 *              - size_t len: length of buffers
 *              - uint8_t cond: condition (0 or 1)
 *************************************************/
void select_bytes(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len,
                  uint8_t cond) {
  // Convert cond to a mask: 0x00 if cond==0, 0xFF if cond==1
  // This is done without branching
  uint8_t mask = (uint8_t)(-(int8_t)cond);

  for (size_t i = 0; i < len; i++) {
    // r[i] = a[i] ^ (mask & (a[i] ^ b[i]))
    // If mask == 0x00: r[i] = a[i]
    // If mask == 0xFF: r[i] = a[i] ^ (a[i] ^ b[i]) = b[i]
    r[i] = a[i] ^ (mask & (a[i] ^ b[i]));
  }
}

/*************************************************
 * Name:        ct_memcmp
 *
 * Description: Constant-time comparison of two byte arrays.
 *              Returns 0 if equal, non-zero otherwise.
 *
 * Arguments:   - const uint8_t *a: first array
 *              - const uint8_t *b: second array
 *              - size_t len: length of arrays
 *
 * Returns:     0 if a == b, non-zero otherwise
 *************************************************/
int ct_memcmp(const uint8_t *a, const uint8_t *b, size_t len) {
  uint8_t r = 0;
  for (size_t i = 0; i < len; i++)
    r |= a[i] ^ b[i];
  return (int)r;
}

/*************************************************
 * Name:        ct_cmov
 *
 * Description: Constant-time conditional move.
 *              Copy len bytes from x to r if b is 1.
 *
 * Arguments:   - uint8_t *r: destination
 *              - const uint8_t *x: source
 *              - size_t len: length
 *              - uint8_t b: condition (0 or 1)
 *************************************************/
void ct_cmov(uint8_t *r, const uint8_t *x, size_t len, uint8_t b) {
  uint8_t mask = (uint8_t)(-(int8_t)b);
  for (size_t i = 0; i < len; i++)
    r[i] ^= mask & (r[i] ^ x[i]);
}
