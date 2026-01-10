/*************************************************
 * Test Kyber KEM
 *
 * A simple test to verify the KEM round-trip works:
 * keygen -> encaps -> decaps -> compare keys
 *************************************************/

#include "../include/kem.h"
#include "../include/params.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  uint8_t pk[KYBER_PUBLICKEYBYTES];
  uint8_t sk[KYBER_SECRETKEYBYTES];
  uint8_t ct[KYBER_CIPHERTEXTBYTES];
  uint8_t ss1[KYBER_SSBYTES]; // Sender's shared secret
  uint8_t ss2[KYBER_SSBYTES]; // Receiver's shared secret
  int i;

  printf("===========================================\n");
  printf("  Kyber-512 Key Encapsulation Test\n");
  printf("===========================================\n\n");

  // Key Generation
  printf("[1] Generating keypair...\n");
  crypto_kem_keypair(pk, sk);
  printf("    Public key:  %d bytes\n", KYBER_PUBLICKEYBYTES);
  printf("    Secret key:  %d bytes\n", KYBER_SECRETKEYBYTES);

  // Encapsulation
  printf("[2] Encapsulating shared secret...\n");
  crypto_kem_enc(ct, ss1, pk);
  printf("    Ciphertext:  %d bytes\n", KYBER_CIPHERTEXTBYTES);
  printf("    Shared key:  %d bytes\n", KYBER_SSBYTES);

  // Decapsulation
  printf("[3] Decapsulating shared secret...\n");
  crypto_kem_dec(ss2, ct, sk);

  // Compare shared secrets
  printf("[4] Comparing shared secrets...\n");
  if (memcmp(ss1, ss2, KYBER_SSBYTES) == 0) {
    printf("    SUCCESS! Shared secrets match.\n\n");
    printf("Shared secret (hex): ");
    for (i = 0; i < KYBER_SSBYTES && i < 16; i++)
      printf("%02x", ss1[i]);
    printf("...\n");
    return 0;
  } else {
    printf("    FAILURE! Shared secrets do NOT match.\n");
    return 1;
  }
}
