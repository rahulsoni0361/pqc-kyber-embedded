#ifndef PLATFORM_H
#define PLATFORM_H

/*************************************************
 * Platform Configuration for Embedded Targets
 *
 * This header adapts Kyber for different MCU platforms:
 *   - STM32F4/F7/H7/L4
 *   - ESP32/ESP32-S2/ESP32-S3
 *   - nRF52832/nRF52840
 *************************************************/

#include <stddef.h>
#include <stdint.h>


/*************************************************
 * Memory Configuration
 *
 * Tune these based on your target's RAM constraints
 *************************************************/

// Stack usage estimates (in bytes) for key operations:
// - crypto_kem_keypair: ~4KB
// - crypto_kem_enc:     ~6KB
// - crypto_kem_dec:     ~6KB
//
// Total peak RAM usage: ~8KB (including keys and ciphertext)

#if defined(KYBER_PLATFORM_STM32)
// STM32 typically has 64KB-512KB SRAM
#define KYBER_STACK_SIZE 8192
#define KYBER_USE_STATIC_ALLOC 1

#elif defined(KYBER_PLATFORM_ESP32)
// ESP32 has ~320KB SRAM available
#define KYBER_STACK_SIZE 8192
#define KYBER_USE_STATIC_ALLOC 1

#elif defined(KYBER_PLATFORM_NRF52)
// nRF52840 has 256KB SRAM, nRF52832 has 64KB
#define KYBER_STACK_SIZE 8192
#define KYBER_USE_STATIC_ALLOC 1

#else
// Desktop - no constraints
#define KYBER_STACK_SIZE 0
#define KYBER_USE_STATIC_ALLOC 0
#endif

/*************************************************
 * Timing / Benchmark Support
 *************************************************/

#if defined(KYBER_PLATFORM_STM32)
// Use DWT cycle counter (Cortex-M4/M7)
#define KYBER_HAS_CYCLE_COUNTER 1

static inline void kyber_cycles_init(void) {
  // Enable DWT cycle counter
  volatile uint32_t *DWT_CTRL = (uint32_t *)0xE0001000;
  volatile uint32_t *DWT_CYCCNT = (uint32_t *)0xE0001004;
  volatile uint32_t *DEMCR = (uint32_t *)0xE000EDFC;
  *DEMCR |= (1 << 24); // Enable trace
  *DWT_CYCCNT = 0;
  *DWT_CTRL |= 1; // Enable cycle counter
}

static inline uint32_t kyber_cycles_read(void) {
  volatile uint32_t *DWT_CYCCNT = (uint32_t *)0xE0001004;
  return *DWT_CYCCNT;
}

#elif defined(KYBER_PLATFORM_ESP32)
// Use CCOUNT register
#define KYBER_HAS_CYCLE_COUNTER 1

static inline void kyber_cycles_init(void) {
  // No init needed on ESP32
}

static inline uint32_t kyber_cycles_read(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0, ccount" : "=a"(ccount));
  return ccount;
}

#elif defined(KYBER_PLATFORM_NRF52)
// Use systick or timer
#define KYBER_HAS_CYCLE_COUNTER 0
// For nRF52, use app_timer or RTC for timing

#else
#define KYBER_HAS_CYCLE_COUNTER 0
#endif

/*************************************************
 * Endianness
 *************************************************/

// All our target platforms are little-endian:
// STM32 (ARM Cortex-M): Little-endian
// ESP32 (Xtensa/RISC-V): Little-endian
// nRF52 (ARM Cortex-M4): Little-endian

#define KYBER_LITTLE_ENDIAN 1

/*************************************************
 * Compiler Attributes
 *************************************************/

#if defined(__GNUC__) || defined(__clang__)
#define KYBER_ALIGN(x) __attribute__((aligned(x)))
#define KYBER_NOINLINE __attribute__((noinline))
#define KYBER_INLINE static inline __attribute__((always_inline))
#define KYBER_UNUSED __attribute__((unused))
#else
#define KYBER_ALIGN(x)
#define KYBER_NOINLINE
#define KYBER_INLINE static inline
#define KYBER_UNUSED
#endif

/*************************************************
 * Debug Output
 *************************************************/

#ifndef KYBER_DEBUG
#define KYBER_DEBUG 0
#endif

#if KYBER_DEBUG
#if defined(KYBER_PLATFORM_STM32)
// Use UART/ITM trace
#include <stdio.h>
#define KYBER_PRINTF(...) printf(__VA_ARGS__)
#elif defined(KYBER_PLATFORM_ESP32)
#include "esp_log.h"
#define KYBER_PRINTF(...) ESP_LOGI("KYBER", __VA_ARGS__)
#elif defined(KYBER_PLATFORM_NRF52)
#include "nrf_log.h"
#define KYBER_PRINTF(...) NRF_LOG_INFO(__VA_ARGS__)
#else
#include <stdio.h>
#define KYBER_PRINTF(...) printf(__VA_ARGS__)
#endif
#else
#define KYBER_PRINTF(...) ((void)0)
#endif

#endif /* PLATFORM_H */
