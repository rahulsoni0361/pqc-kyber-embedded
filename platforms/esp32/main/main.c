/*************************************************
 * Kyber ESP32 Demo - Main Entry Point
 *
 * This demonstrates post-quantum key exchange on ESP32
 * using the CRYSTALS-Kyber algorithm
 *************************************************/

#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>


#include "kem.h"
#include "params.h"

static const char *TAG = "KYBER";

/*************************************************
 * Run Kyber KEM Demo
 *************************************************/
static void kyber_demo(void) {
  uint8_t pk[KYBER_PUBLICKEYBYTES];
  uint8_t sk[KYBER_SECRETKEYBYTES];
  uint8_t ct[KYBER_CIPHERTEXTBYTES];
  uint8_t ss1[KYBER_SSBYTES];
  uint8_t ss2[KYBER_SSBYTES];
  uint32_t start_time, end_time;

  ESP_LOGI(TAG, "===========================================");
  ESP_LOGI(TAG, "  Kyber-512 Post-Quantum KEM Demo");
  ESP_LOGI(TAG, "===========================================");

  // Print sizes
  ESP_LOGI(TAG, "Key sizes:");
  ESP_LOGI(TAG, "  Public Key:  %d bytes", KYBER_PUBLICKEYBYTES);
  ESP_LOGI(TAG, "  Secret Key:  %d bytes", KYBER_SECRETKEYBYTES);
  ESP_LOGI(TAG, "  Ciphertext:  %d bytes", KYBER_CIPHERTEXTBYTES);
  ESP_LOGI(TAG, "  Shared Key:  %d bytes", KYBER_SSBYTES);

  // Free heap before
  ESP_LOGI(TAG, "Free heap: %lu bytes",
           (unsigned long)esp_get_free_heap_size());

  // Key Generation
  ESP_LOGI(TAG, "[1] Generating keypair...");
  start_time = esp_timer_get_time();
  crypto_kem_keypair(pk, sk);
  end_time = esp_timer_get_time();
  ESP_LOGI(TAG, "    Time: %lu us", (unsigned long)(end_time - start_time));

  // Encapsulation
  ESP_LOGI(TAG, "[2] Encapsulating shared secret...");
  start_time = esp_timer_get_time();
  crypto_kem_enc(ct, ss1, pk);
  end_time = esp_timer_get_time();
  ESP_LOGI(TAG, "    Time: %lu us", (unsigned long)(end_time - start_time));

  // Decapsulation
  ESP_LOGI(TAG, "[3] Decapsulating shared secret...");
  start_time = esp_timer_get_time();
  crypto_kem_dec(ss2, ct, sk);
  end_time = esp_timer_get_time();
  ESP_LOGI(TAG, "    Time: %lu us", (unsigned long)(end_time - start_time));

  // Verify
  ESP_LOGI(TAG, "[4] Verifying shared secrets...");
  if (memcmp(ss1, ss2, KYBER_SSBYTES) == 0) {
    ESP_LOGI(TAG, "    SUCCESS! Shared secrets match.");

    // Print first 16 bytes of shared secret
    char hex[33];
    for (int i = 0; i < 16; i++) {
      sprintf(&hex[i * 2], "%02x", ss1[i]);
    }
    ESP_LOGI(TAG, "    Shared secret: %s...", hex);
  } else {
    ESP_LOGE(TAG, "    FAILURE! Shared secrets do NOT match.");
  }

  // Free heap after
  ESP_LOGI(TAG, "Free heap: %lu bytes",
           (unsigned long)esp_get_free_heap_size());
}

/*************************************************
 * Main Entry Point
 *************************************************/
void app_main(void) {
  ESP_LOGI(TAG, "Starting Kyber Demo...");
  ESP_LOGI(TAG, "Chip: %s, Cores: %d", CONFIG_IDF_TARGET,
           esp_get_chip_info(NULL));

  // Run the demo
  kyber_demo();

  ESP_LOGI(TAG, "Demo complete!");

  // Keep running (or deep sleep)
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
