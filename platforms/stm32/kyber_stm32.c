/*************************************************
 * Kyber STM32 Demo - Main Entry Point
 *
 * This demonstrates post-quantum key exchange on STM32
 * Tested on: STM32F407, STM32F746, STM32H743
 *
 * Requirements:
 *   - STM32 HAL library
 *   - RNG peripheral enabled
 *   - UART for debug output (optional)
 *************************************************/

#include "main.h"
#include <stdio.h>
#include <string.h>

#include "kem.h"
#include "params.h"

/* RNG Handle - must be initialized in CubeMX */
extern RNG_HandleTypeDef hrng;

/* UART Handle for debug output (optional) */
#ifdef USE_UART_DEBUG
extern UART_HandleTypeDef huart2;
#endif

/*************************************************
 * Retarget printf to UART (optional)
 *************************************************/
#ifdef USE_UART_DEBUG
int __io_putchar(int ch) {
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
#endif

/*************************************************
 * DWT Cycle Counter Functions
 *************************************************/
static void dwt_init(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t dwt_get_cycles(void) { return DWT->CYCCNT; }

/*************************************************
 * Run Kyber KEM Benchmark
 *************************************************/
void kyber_benchmark(void) {
  uint8_t pk[KYBER_PUBLICKEYBYTES];
  uint8_t sk[KYBER_SECRETKEYBYTES];
  uint8_t ct[KYBER_CIPHERTEXTBYTES];
  uint8_t ss1[KYBER_SSBYTES];
  uint8_t ss2[KYBER_SSBYTES];
  uint32_t start, keygen_cycles, encaps_cycles, decaps_cycles;

  printf("===========================================\r\n");
  printf("  Kyber-512 STM32 Benchmark\r\n");
  printf("===========================================\r\n\n");

  // Print configuration
  printf("Configuration:\r\n");
  printf("  KYBER_K:         %d\r\n", KYBER_K);
  printf("  Public Key:      %d bytes\r\n", KYBER_PUBLICKEYBYTES);
  printf("  Secret Key:      %d bytes\r\n", KYBER_SECRETKEYBYTES);
  printf("  Ciphertext:      %d bytes\r\n", KYBER_CIPHERTEXTBYTES);
  printf("  Shared Secret:   %d bytes\r\n", KYBER_SSBYTES);
  printf("  CPU Clock:       %lu MHz\r\n\r\n", HAL_RCC_GetHCLKFreq() / 1000000);

  // Initialize cycle counter
  dwt_init();

  // Key Generation
  printf("[1] Key Generation...\r\n");
  start = dwt_get_cycles();
  crypto_kem_keypair(pk, sk);
  keygen_cycles = dwt_get_cycles() - start;
  printf("    Cycles: %lu\r\n", keygen_cycles);
  printf("    Time:   %.2f ms\r\n\r\n",
         (float)keygen_cycles / (HAL_RCC_GetHCLKFreq() / 1000));

  // Encapsulation
  printf("[2] Encapsulation...\r\n");
  start = dwt_get_cycles();
  crypto_kem_enc(ct, ss1, pk);
  encaps_cycles = dwt_get_cycles() - start;
  printf("    Cycles: %lu\r\n", encaps_cycles);
  printf("    Time:   %.2f ms\r\n\r\n",
         (float)encaps_cycles / (HAL_RCC_GetHCLKFreq() / 1000));

  // Decapsulation
  printf("[3] Decapsulation...\r\n");
  start = dwt_get_cycles();
  crypto_kem_dec(ss2, ct, sk);
  decaps_cycles = dwt_get_cycles() - start;
  printf("    Cycles: %lu\r\n", decaps_cycles);
  printf("    Time:   %.2f ms\r\n\r\n",
         (float)decaps_cycles / (HAL_RCC_GetHCLKFreq() / 1000));

  // Verification
  printf("[4] Verification...\r\n");
  if (memcmp(ss1, ss2, KYBER_SSBYTES) == 0) {
    printf("    SUCCESS! Shared secrets match.\r\n\r\n");
  } else {
    printf("    FAILURE! Shared secrets do NOT match.\r\n\r\n");
  }

  // Summary
  printf("===========================================\r\n");
  printf("  Summary\r\n");
  printf("===========================================\r\n");
  printf("Total Cycles: %lu\r\n",
         keygen_cycles + encaps_cycles + decaps_cycles);
  printf("Total Time:   %.2f ms\r\n",
         (float)(keygen_cycles + encaps_cycles + decaps_cycles) /
             (HAL_RCC_GetHCLKFreq() / 1000));

  // Print shared secret (first 16 bytes)
  printf("\nShared Secret: ");
  for (int i = 0; i < 16; i++) {
    printf("%02X", ss1[i]);
  }
  printf("...\r\n");
}

/*************************************************
 * LED Blink to indicate success
 *************************************************/
void blink_success(void) {
  for (int i = 0; i < 5; i++) {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); // Adjust for your board
    HAL_Delay(200);
  }
}

/*************************************************
 * Main Function
 *
 * Note: HAL_Init(), SystemClock_Config(), and peripheral
 * initialization should be done in CubeMX-generated code
 *************************************************/
void kyber_stm32_main(void) {
  printf("\r\n\r\n");
  printf("STM32 Kyber Demo Starting...\r\n");

  // Wait for UART to be ready
  HAL_Delay(100);

  // Run benchmark
  kyber_benchmark();

  // Indicate success
  blink_success();

  printf("Demo complete. Entering idle loop.\r\n");
}
