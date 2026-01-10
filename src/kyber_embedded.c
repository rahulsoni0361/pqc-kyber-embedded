/*************************************************
 * Kyber KEM Embedded Benchmark
 *
 * This file demonstrates Kyber on embedded platforms:
 *   - STM32F4/F7/H7
 *   - ESP32/ESP32-S3
 *   - nRF52840
 *
 * It performs key generation, encapsulation, and decapsulation
 * while measuring execution time and memory usage.
 *************************************************/

#include "../include/kem.h"
#include "../include/params.h"
#include "../include/platform.h"
#include "../include/randombytes.h"
#include <stdint.h>
#include <string.h>

/*************************************************
 * Benchmark Results Structure
 *************************************************/
typedef struct {
  uint32_t keygen_cycles;
  uint32_t encaps_cycles;
  uint32_t decaps_cycles;
  uint32_t total_cycles;
  int test_passed;
} kyber_benchmark_t;

/*************************************************
 * Static buffers for embedded (no malloc)
 *************************************************/
#if KYBER_USE_STATIC_ALLOC
static uint8_t pk_buf[KYBER_PUBLICKEYBYTES];
static uint8_t sk_buf[KYBER_SECRETKEYBYTES];
static uint8_t ct_buf[KYBER_CIPHERTEXTBYTES];
static uint8_t ss1_buf[KYBER_SSBYTES];
static uint8_t ss2_buf[KYBER_SSBYTES];
#endif

/*************************************************
 * Name:        kyber_benchmark
 *
 * Description: Run a full KEM benchmark
 *
 * Arguments:   - kyber_benchmark_t *result: benchmark results
 *
 * Returns:     0 on success, -1 on failure
 *************************************************/
int kyber_benchmark(kyber_benchmark_t *result) {
  uint8_t *pk, *sk, *ct, *ss1, *ss2;
  uint32_t start, end;

#if KYBER_USE_STATIC_ALLOC
  pk = pk_buf;
  sk = sk_buf;
  ct = ct_buf;
  ss1 = ss1_buf;
  ss2 = ss2_buf;
#else
  uint8_t pk_local[KYBER_PUBLICKEYBYTES];
  uint8_t sk_local[KYBER_SECRETKEYBYTES];
  uint8_t ct_local[KYBER_CIPHERTEXTBYTES];
  uint8_t ss1_local[KYBER_SSBYTES];
  uint8_t ss2_local[KYBER_SSBYTES];
  pk = pk_local;
  sk = sk_local;
  ct = ct_local;
  ss1 = ss1_local;
  ss2 = ss2_local;
#endif

  memset(result, 0, sizeof(*result));

#if KYBER_HAS_CYCLE_COUNTER
  kyber_cycles_init();
#endif

  // Key Generation
  KYBER_PRINTF("Generating keypair...\n");
#if KYBER_HAS_CYCLE_COUNTER
  start = kyber_cycles_read();
#endif
  crypto_kem_keypair(pk, sk);
#if KYBER_HAS_CYCLE_COUNTER
  end = kyber_cycles_read();
  result->keygen_cycles = end - start;
#endif

  // Encapsulation
  KYBER_PRINTF("Encapsulating...\n");
#if KYBER_HAS_CYCLE_COUNTER
  start = kyber_cycles_read();
#endif
  crypto_kem_enc(ct, ss1, pk);
#if KYBER_HAS_CYCLE_COUNTER
  end = kyber_cycles_read();
  result->encaps_cycles = end - start;
#endif

  // Decapsulation
  KYBER_PRINTF("Decapsulating...\n");
#if KYBER_HAS_CYCLE_COUNTER
  start = kyber_cycles_read();
#endif
  crypto_kem_dec(ss2, ct, sk);
#if KYBER_HAS_CYCLE_COUNTER
  end = kyber_cycles_read();
  result->decaps_cycles = end - start;
#endif

  // Verify shared secrets match
  result->test_passed = (memcmp(ss1, ss2, KYBER_SSBYTES) == 0);
  result->total_cycles =
      result->keygen_cycles + result->encaps_cycles + result->decaps_cycles;

  return result->test_passed ? 0 : -1;
}

/*************************************************
 * Name:        kyber_memory_info
 *
 * Description: Print memory usage information
 *************************************************/
void kyber_memory_info(void) {
  KYBER_PRINTF("=== Kyber Memory Usage ===\n");
  KYBER_PRINTF("Public Key:  %d bytes\n", KYBER_PUBLICKEYBYTES);
  KYBER_PRINTF("Secret Key:  %d bytes\n", KYBER_SECRETKEYBYTES);
  KYBER_PRINTF("Ciphertext:  %d bytes\n", KYBER_CIPHERTEXTBYTES);
  KYBER_PRINTF("Shared Key:  %d bytes\n", KYBER_SSBYTES);
  KYBER_PRINTF("Total Keys:  %d bytes\n",
               KYBER_PUBLICKEYBYTES + KYBER_SECRETKEYBYTES +
                   KYBER_CIPHERTEXTBYTES + KYBER_SSBYTES);

#if KYBER_K == 2
  KYBER_PRINTF("Security:    Kyber-512 (NIST Level 1)\n");
#elif KYBER_K == 3
  KYBER_PRINTF("Security:    Kyber-768 (NIST Level 3)\n");
#elif KYBER_K == 4
  KYBER_PRINTF("Security:    Kyber-1024 (NIST Level 5)\n");
#endif
}

/*************************************************
 * Platform-specific main functions
 *************************************************/

#if defined(KYBER_PLATFORM_STM32)
/* For STM32, this would be called from main() after HAL_Init() */

void kyber_stm32_demo(void) {
  kyber_benchmark_t result;

  kyber_memory_info();

  if (kyber_benchmark(&result) == 0) {
    KYBER_PRINTF("\n=== Benchmark Results ===\n");
    KYBER_PRINTF("KeyGen:  %lu cycles\n", (unsigned long)result.keygen_cycles);
    KYBER_PRINTF("Encaps:  %lu cycles\n", (unsigned long)result.encaps_cycles);
    KYBER_PRINTF("Decaps:  %lu cycles\n", (unsigned long)result.decaps_cycles);
    KYBER_PRINTF("Total:   %lu cycles\n", (unsigned long)result.total_cycles);
    KYBER_PRINTF("Status:  PASSED\n");
  } else {
    KYBER_PRINTF("Status:  FAILED\n");
  }
}

#elif defined(KYBER_PLATFORM_ESP32)
/* ESP-IDF task entry point */

void kyber_esp32_task(void *pvParameters) {
  kyber_benchmark_t result;

  kyber_memory_info();

  while (1) {
    if (kyber_benchmark(&result) == 0) {
      KYBER_PRINTF("KeyGen: %u cycles", result.keygen_cycles);
      KYBER_PRINTF("Encaps: %u cycles", result.encaps_cycles);
      KYBER_PRINTF("Decaps: %u cycles", result.decaps_cycles);
      KYBER_PRINTF("Test PASSED");
    } else {
      KYBER_PRINTF("Test FAILED");
    }

    // Wait before next iteration
    // vTaskDelay(pdMS_TO_TICKS(5000));
    break; // Run once for now
  }

  // vTaskDelete(NULL);
}

#elif defined(KYBER_PLATFORM_NRF52)
/* nRF52 main entry */

void kyber_nrf52_demo(void) {
  kyber_benchmark_t result;

  kyber_memory_info();

  if (kyber_benchmark(&result) == 0) {
    KYBER_PRINTF("Kyber KEM Test PASSED");
  } else {
    KYBER_PRINTF("Kyber KEM Test FAILED");
  }
}

#else
/* Desktop/generic main */

#include <stdio.h>

int main(void) {
  kyber_benchmark_t result;

  printf("============================================\n");
  printf("  Kyber KEM Embedded Benchmark\n");
  printf("============================================\n\n");

  // Print memory info
  printf("=== Memory Usage ===\n");
  printf("Public Key:  %d bytes\n", KYBER_PUBLICKEYBYTES);
  printf("Secret Key:  %d bytes\n", KYBER_SECRETKEYBYTES);
  printf("Ciphertext:  %d bytes\n", KYBER_CIPHERTEXTBYTES);
  printf("Shared Key:  %d bytes\n", KYBER_SSBYTES);
  printf("Total:       %d bytes\n\n",
         KYBER_PUBLICKEYBYTES + KYBER_SECRETKEYBYTES + KYBER_CIPHERTEXTBYTES +
             KYBER_SSBYTES);

  // Run benchmark
  printf("=== Running Benchmark ===\n");
  if (kyber_benchmark(&result) == 0) {
    printf("\nResult: PASSED\n");
    printf("Shared secrets match!\n");

#if KYBER_HAS_CYCLE_COUNTER
    printf("\n=== Cycle Counts ===\n");
    printf("KeyGen:  %u cycles\n", result.keygen_cycles);
    printf("Encaps:  %u cycles\n", result.encaps_cycles);
    printf("Decaps:  %u cycles\n", result.decaps_cycles);
    printf("Total:   %u cycles\n", result.total_cycles);
#endif
  } else {
    printf("\nResult: FAILED\n");
    printf("Shared secrets do NOT match!\n");
    return 1;
  }

  return 0;
}

#endif
