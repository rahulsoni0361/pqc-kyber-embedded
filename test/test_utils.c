#include "unity.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>


// Implementation of select_bytes for testing
void select_bytes(uint8_t *r, const uint8_t *a, const uint8_t *b, size_t len,
                  uint8_t cond) {
  uint8_t mask = -cond; // 0x00 if cond=0, 0xFF if cond=1
  for (size_t i = 0; i < len; i++) {
    r[i] = a[i] ^ (mask & (a[i] ^ b[i]));
  }
}

void setUp(void) {}
void tearDown(void) {}

void test_select_bytes_chooses_a_when_cond_is_zero(void) {
  uint8_t a[4] = {0x01, 0x02, 0x03, 0x04};
  uint8_t b[4] = {0xFF, 0xEE, 0xDD, 0xCC};
  uint8_t r[4] = {0};

  select_bytes(r, a, b, 4, 0);

  TEST_ASSERT_EQUAL_HEX8_ARRAY(a, r, 4);
}

void test_select_bytes_chooses_b_when_cond_is_one(void) {
  uint8_t a[4] = {0x01, 0x02, 0x03, 0x04};
  uint8_t b[4] = {0xFF, 0xEE, 0xDD, 0xCC};
  uint8_t r[4] = {0};

  select_bytes(r, a, b, 4, 1);

  TEST_ASSERT_EQUAL_HEX8_ARRAY(b, r, 4);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_select_bytes_chooses_a_when_cond_is_zero);
  RUN_TEST(test_select_bytes_chooses_b_when_cond_is_one);
  return UNITY_END();
}
