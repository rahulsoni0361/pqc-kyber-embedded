#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include <stddef.h>
#include <stdint.h>

/*************************************************
 * Random Number Generator Abstraction
 *
 * Each platform must implement randombytes() using
 * its hardware RNG or a secure software DRBG.
 *
 * Supported platforms:
 *   - STM32 (HAL_RNG)
 *   - ESP32 (esp_random)
 *   - nRF52 (nrf_crypto_rng)
 *   - Desktop (os.urandom or OpenSSL)
 *************************************************/

/**
 * @brief Fill buffer with cryptographically secure random bytes
 *
 * @param out   Output buffer
 * @param len   Number of bytes to generate
 * @return      0 on success, -1 on failure
 */
int randombytes(uint8_t *out, size_t len);

/*************************************************
 * Platform Detection
 *************************************************/

#if defined(STM32F4) || defined(STM32F7) || defined(STM32H7) || defined(STM32L4)
#define KYBER_PLATFORM_STM32
#elif defined(ESP32) || defined(ESP_PLATFORM)
#define KYBER_PLATFORM_ESP32
#elif defined(NRF52) || defined(NRF52840_XXAA) || defined(NRF52832_XXAA)
#define KYBER_PLATFORM_NRF52
#elif defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#define KYBER_PLATFORM_DESKTOP
#else
#define KYBER_PLATFORM_GENERIC
#endif

#endif /* RANDOMBYTES_H */
