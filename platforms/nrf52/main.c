/*************************************************
 * Kyber nRF52 Demo - Main Entry Point
 *
 * This demonstrates post-quantum key exchange on nRF52
 * Tested on: nRF52840-DK, nRF52832-DK
 *
 * Requirements:
 *   - nRF5 SDK 17.x or nRF Connect SDK
 *   - nrf_crypto module enabled
 *   - RTT or UART for debug output
 *************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_crypto.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#include "kem.h"
#include "params.h"

/* LED definitions for nRF52840-DK */
#define LED_1 NRF_GPIO_PIN_MAP(0, 13)
#define LED_2 NRF_GPIO_PIN_MAP(0, 14)
#define LED_3 NRF_GPIO_PIN_MAP(0, 15)
#define LED_4 NRF_GPIO_PIN_MAP(0, 16)

/*************************************************
 * Timer for benchmarking
 *************************************************/
static uint32_t benchmark_start_time;
static uint32_t benchmark_end_time;

static void timer_init(void) {
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

static inline uint32_t get_time_us(void) {
  return app_timer_cnt_get() * 1000000 / APP_TIMER_CLOCK_FREQ;
}

/*************************************************
 * LED indication
 *************************************************/
static void leds_init(void) {
  nrf_gpio_cfg_output(LED_1);
  nrf_gpio_cfg_output(LED_2);
  nrf_gpio_cfg_output(LED_3);
  nrf_gpio_cfg_output(LED_4);

  // Turn all LEDs off (active low on DK)
  nrf_gpio_pin_set(LED_1);
  nrf_gpio_pin_set(LED_2);
  nrf_gpio_pin_set(LED_3);
  nrf_gpio_pin_set(LED_4);
}

static void led_on(uint32_t led) { nrf_gpio_pin_clear(led); }

static void led_off(uint32_t led) { nrf_gpio_pin_set(led); }

/*************************************************
 * Logging initialization
 *************************************************/
static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/*************************************************
 * Crypto initialization
 *************************************************/
static void crypto_init(void) {
  ret_code_t err_code = nrf_crypto_init();
  APP_ERROR_CHECK(err_code);

  err_code = nrf_crypto_rng_init(NULL, NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_INFO("nrf_crypto initialized");
}

/*************************************************
 * Run Kyber Benchmark
 *************************************************/
static void kyber_benchmark(void) {
  uint8_t pk[KYBER_PUBLICKEYBYTES];
  uint8_t sk[KYBER_SECRETKEYBYTES];
  uint8_t ct[KYBER_CIPHERTEXTBYTES];
  uint8_t ss1[KYBER_SSBYTES];
  uint8_t ss2[KYBER_SSBYTES];
  uint32_t keygen_time, encaps_time, decaps_time;

  NRF_LOG_INFO("===========================================");
  NRF_LOG_INFO("  Kyber-512 nRF52 Benchmark");
  NRF_LOG_INFO("===========================================");
  NRF_LOG_FLUSH();

  // Print configuration
  NRF_LOG_INFO("Configuration:");
  NRF_LOG_INFO("  Public Key:  %d bytes", KYBER_PUBLICKEYBYTES);
  NRF_LOG_INFO("  Secret Key:  %d bytes", KYBER_SECRETKEYBYTES);
  NRF_LOG_INFO("  Ciphertext:  %d bytes", KYBER_CIPHERTEXTBYTES);
  NRF_LOG_INFO("  Shared Key:  %d bytes", KYBER_SSBYTES);
  NRF_LOG_FLUSH();

  // Key Generation
  NRF_LOG_INFO("[1] Key Generation...");
  led_on(LED_1);
  benchmark_start_time = get_time_us();
  crypto_kem_keypair(pk, sk);
  benchmark_end_time = get_time_us();
  keygen_time = benchmark_end_time - benchmark_start_time;
  NRF_LOG_INFO("    Time: %u us", keygen_time);
  led_off(LED_1);
  NRF_LOG_FLUSH();

  // Encapsulation
  NRF_LOG_INFO("[2] Encapsulation...");
  led_on(LED_2);
  benchmark_start_time = get_time_us();
  crypto_kem_enc(ct, ss1, pk);
  benchmark_end_time = get_time_us();
  encaps_time = benchmark_end_time - benchmark_start_time;
  NRF_LOG_INFO("    Time: %u us", encaps_time);
  led_off(LED_2);
  NRF_LOG_FLUSH();

  // Decapsulation
  NRF_LOG_INFO("[3] Decapsulation...");
  led_on(LED_3);
  benchmark_start_time = get_time_us();
  crypto_kem_dec(ss2, ct, sk);
  benchmark_end_time = get_time_us();
  decaps_time = benchmark_end_time - benchmark_start_time;
  NRF_LOG_INFO("    Time: %u us", decaps_time);
  led_off(LED_3);
  NRF_LOG_FLUSH();

  // Verification
  NRF_LOG_INFO("[4] Verification...");
  if (memcmp(ss1, ss2, KYBER_SSBYTES) == 0) {
    NRF_LOG_INFO("    SUCCESS! Shared secrets match.");
    led_on(LED_4); // Success indicator
  } else {
    NRF_LOG_ERROR("    FAILURE!");
    // Blink all LEDs for failure
    for (int i = 0; i < 10; i++) {
      nrf_gpio_pin_toggle(LED_1);
      nrf_gpio_pin_toggle(LED_2);
      nrf_gpio_pin_toggle(LED_3);
      nrf_gpio_pin_toggle(LED_4);
      nrf_delay_ms(100);
    }
  }
  NRF_LOG_FLUSH();

  // Summary
  NRF_LOG_INFO("===========================================");
  NRF_LOG_INFO("  Total Time: %u us", keygen_time + encaps_time + decaps_time);
  NRF_LOG_INFO("===========================================");

  // Print shared secret (first 16 bytes)
  NRF_LOG_INFO("Shared Secret:");
  NRF_LOG_HEXDUMP_INFO(ss1, 16);
  NRF_LOG_FLUSH();
}

/*************************************************
 * Main Entry Point
 *************************************************/
int main(void) {
  // Initialize
  log_init();
  timer_init();
  leds_init();
  crypto_init();

  NRF_LOG_INFO("nRF52 Kyber Demo Starting...");
  NRF_LOG_FLUSH();

  nrf_delay_ms(1000); // Wait for terminal to connect

  // Run benchmark
  kyber_benchmark();

  NRF_LOG_INFO("Demo complete!");
  NRF_LOG_FLUSH();

  // Idle loop
  while (true) {
    NRF_LOG_FLUSH();
    __WFE();
  }
}
