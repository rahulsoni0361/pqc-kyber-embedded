#ifndef FIPS202_H
#define FIPS202_H

#include <stddef.h>
#include <stdint.h>

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136
#define SHA3_256_RATE 136
#define SHA3_512_RATE 72

// Context for incremental hashing
typedef struct {
  uint64_t s[25];
  unsigned int output_len;
} keccak_state;

// SHA3-256
void sha3_256(uint8_t *output, const uint8_t *input, size_t inlen);

// SHA3-512
void sha3_512(uint8_t *output, const uint8_t *input, size_t inlen);

// SHAKE-128
void shake128(uint8_t *output, size_t outlen, const uint8_t *input,
              size_t inlen);

// SHAKE-256
void shake256(uint8_t *output, size_t outlen, const uint8_t *input,
              size_t inlen);

// Incremental SHAKE-128 (useful for generating matrix A)
void shake128_absorb(keccak_state *state, const uint8_t *input, size_t inlen);
void shake128_squeezeblocks(uint8_t *output, size_t nblocks,
                            keccak_state *state);
void shake128_ctx_release(keccak_state *state);

// Incremental SHAKE-256
void shake256_absorb(keccak_state *state, const uint8_t *input, size_t inlen);
void shake256_squeezeblocks(uint8_t *output, size_t nblocks,
                            keccak_state *state);
void shake256_ctx_release(keccak_state *state);

#endif /* FIPS202_H */
