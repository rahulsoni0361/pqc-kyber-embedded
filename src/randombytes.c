/*************************************************
 * Random Number Generator Implementations
 *
 * Platform-specific implementations for:
 *   - STM32 (using HAL RNG peripheral)
 *   - ESP32 (using hardware RNG)
 *   - nRF52 (using cryptocell or TRNG)
 *   - Desktop (using OS random for production, LFSR for testing)
 *************************************************/

#include "../include/randombytes.h"
#include <stddef.h>
#include <stdint.h>

/*************************************************
 * STM32 Implementation
 * Requires: STM32 HAL with RNG enabled
 *************************************************/
#if defined(KYBER_PLATFORM_STM32)

#include "stm32f4xx_hal.h" // Adjust for your STM32 family

extern RNG_HandleTypeDef hrng; // Must be initialized in main.c

int randombytes(uint8_t *out, size_t len) {
  uint32_t random_word;
  size_t i = 0;

  while (i < len) {
    if (HAL_RNG_GenerateRandomNumber(&hrng, &random_word) != HAL_OK) {
      return -1;
    }

    // Copy up to 4 bytes from the random word
    size_t bytes_to_copy = (len - i < 4) ? (len - i) : 4;
    for (size_t j = 0; j < bytes_to_copy; j++) {
      out[i++] = (uint8_t)(random_word >> (8 * j));
    }
  }

  return 0;
}

/*************************************************
 * ESP32 Implementation
 * Uses hardware RNG (no additional setup needed)
 *************************************************/
#elif defined(KYBER_PLATFORM_ESP32)

#include "esp_random.h"

int randombytes(uint8_t *out, size_t len) {
  esp_fill_random(out, len);
  return 0;
}

/*************************************************
 * nRF52 Implementation
 * Uses nRF5 SDK RNG or CryptoCell
 *************************************************/
#elif defined(KYBER_PLATFORM_NRF52)

#include "nrf_crypto_rng.h"

int randombytes(uint8_t *out, size_t len) {
  ret_code_t ret = nrf_crypto_rng_vector_generate(out, len);
  return (ret == NRF_SUCCESS) ? 0 : -1;
}

/*************************************************
 * Desktop/Generic Implementation
 * For production on Desktop: use OS random
 * For testing: use deterministic LFSR
 *************************************************/
#else

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Use deterministic LFSR seeded with time for testing
// For production, use OS-specific secure random

static uint32_t lfsr_state = 0;
static int lfsr_initialized = 0;

static void lfsr_init(void) {
  if (!lfsr_initialized) {
    lfsr_state = (uint32_t)time(NULL) ^ 0xDEADBEEF;
    if (lfsr_state == 0)
      lfsr_state = 0xDEADBEEF;
    lfsr_initialized = 1;
  }
}

static uint32_t lfsr_next(void) {
  // Galois LFSR with maximal period polynomial
  uint32_t bit = lfsr_state & 1;
  lfsr_state >>= 1;
  if (bit) {
    lfsr_state ^= 0xB4BCD35C; // Maximal polynomial
  }
  return lfsr_state;
}

int randombytes(uint8_t *out, size_t len) {
  lfsr_init();

  size_t i = 0;
  while (i < len) {
    uint32_t r = lfsr_next();
    size_t bytes_to_copy = (len - i < 4) ? (len - i) : 4;
    for (size_t j = 0; j < bytes_to_copy; j++) {
      out[i++] = (uint8_t)(r >> (8 * j));
    }
  }

  return 0;
}

// Allow explicit seeding for deterministic tests
void randombytes_seed(uint32_t seed) {
  lfsr_state = seed ? seed : 0xDEADBEEF;
  lfsr_initialized = 1;
}

#endif
