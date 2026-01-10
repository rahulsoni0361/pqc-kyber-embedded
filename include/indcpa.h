#ifndef INDCPA_H
#define INDCPA_H

#include "params.h"
#include <stdint.h>


/*************************************************
 * Name:        indcpa_keypair
 *
 * Description: Generates public and private key for the CPA-secure
 *              public-key encryption scheme underlying Kyber
 *
 * Arguments:   - uint8_t *pk: pointer to output public key
 *                             (of length KYBER_PUBLICKEYBYTES bytes)
 *              - uint8_t *sk: pointer to output private key
 *                             (of length KYBER_SECRETKEYBYTES bytes)
 **************************************************/
void indcpa_keypair(uint8_t pk[KYBER_PUBLICKEYBYTES],
                    uint8_t sk[KYBER_SECRETKEYBYTES]);

/*************************************************
 * Name:        indcpa_enc
 *
 * Description: Encryption function of the CPA-secure
 *              public-key encryption scheme underlying Kyber.
 *
 * Arguments:   - uint8_t *c: pointer to output ciphertext
 *                            (of length KYBER_CIPHERTEXTBYTES bytes)
 *              - const uint8_t *m: pointer to input message
 *                                  (of length KYBER_SYMBYTES bytes)
 *              - const uint8_t *pk: pointer to input public key
 *                                   (of length KYBER_PUBLICKEYBYTES bytes)
 *              - const uint8_t *coins: pointer to input random coins
 *                                      used as seed (of length KYBER_SYMBYTES
 * bytes) to deterministically generate all randomness
 **************************************************/
void indcpa_enc(uint8_t c[KYBER_CIPHERTEXTBYTES],
                const uint8_t m[KYBER_SYMBYTES],
                const uint8_t pk[KYBER_PUBLICKEYBYTES],
                const uint8_t coins[KYBER_SYMBYTES]);

/*************************************************
 * Name:        indcpa_dec
 *
 * Description: Decryption function of the CPA-secure
 *              public-key encryption scheme underlying Kyber.
 *
 * Arguments:   - uint8_t *m: pointer to output decrypted message
 *                            (of length KYBER_SYMBYTES bytes)
 *              - const uint8_t *c: pointer to input ciphertext
 *                                  (of length KYBER_CIPHERTEXTBYTES bytes)
 *              - const uint8_t *sk: pointer to input secret key
 *                                   (of length KYBER_SECRETKEYBYTES bytes)
 **************************************************/
void indcpa_dec(uint8_t m[KYBER_SYMBYTES],
                const uint8_t c[KYBER_CIPHERTEXTBYTES],
                const uint8_t sk[KYBER_SECRETKEYBYTES]);

#endif /* INDCPA_H */
