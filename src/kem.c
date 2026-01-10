/*************************************************
 * Kyber Key Encapsulation Mechanism (KEM)
 *
 * CCA-secure KEM built using Fujisaki-Okamoto transform
 * on top of the IND-CPA secure PKE
 *
 * Implements Algorithms 7, 8, 9 from the Kyber specification
 *************************************************/

#include "../include/kem.h"
#include "../include/fips202.h"
#include "../include/indcpa.h"
#include "../include/params.h"
#include "../include/randombytes.h"
#include "../include/utils.h"
#include <stdint.h>
#include <string.h>


/*************************************************
 * Name:        crypto_kem_keypair
 *
 * Description: Generates public and private key for CCA KEM
 *
 * Algorithm 7 from Kyber spec
 *************************************************/
int crypto_kem_keypair(uint8_t pk[KYBER_PUBLICKEYBYTES],
                       uint8_t sk[KYBER_SECRETKEYBYTES]) {
  // Generate IND-CPA keypair
  indcpa_keypair(pk, sk);

  // Append public key to secret key
  memcpy(sk + KYBER_POLYVECBYTES, pk, KYBER_PUBLICKEYBYTES);

  // Append H(pk) to secret key
  sha3_256(sk + KYBER_SECRETKEYBYTES - 2 * KYBER_SYMBYTES, pk,
           KYBER_PUBLICKEYBYTES);

  // Append random z to secret key (for implicit rejection)
  randombytes(sk + KYBER_SECRETKEYBYTES - KYBER_SYMBYTES, KYBER_SYMBYTES);

  return 0;
}

/*************************************************
 * Name:        crypto_kem_enc
 *
 * Description: Generates shared secret and ciphertext
 *
 * Algorithm 8 from Kyber spec
 *************************************************/
int crypto_kem_enc(uint8_t ct[KYBER_CIPHERTEXTBYTES], uint8_t ss[KYBER_SSBYTES],
                   const uint8_t pk[KYBER_PUBLICKEYBYTES]) {
  uint8_t buf[2 * KYBER_SYMBYTES];
  uint8_t kr[2 * KYBER_SYMBYTES]; // (K_bar, r)

  // Generate random message m
  randombytes(buf, KYBER_SYMBYTES);

  // Hash m to get m_hash (the "hash of shame")
  sha3_256(buf, buf, KYBER_SYMBYTES);

  // Compute (K_bar, r) = G(m || H(pk))
  sha3_256(buf + KYBER_SYMBYTES, pk, KYBER_PUBLICKEYBYTES);
  sha3_512(kr, buf, 2 * KYBER_SYMBYTES);

  // Encrypt m using r as randomness
  indcpa_enc(ct, buf, pk, kr + KYBER_SYMBYTES);

  // Compute shared key K = KDF(K_bar || H(c))
  sha3_256(kr + KYBER_SYMBYTES, ct, KYBER_CIPHERTEXTBYTES);
  shake256(ss, KYBER_SSBYTES, kr, 2 * KYBER_SYMBYTES);

  return 0;
}

/*************************************************
 * Name:        crypto_kem_dec
 *
 * Description: Generates shared secret from ciphertext
 *
 * Algorithm 9 from Kyber spec (with implicit rejection)
 *************************************************/
int crypto_kem_dec(uint8_t ss[KYBER_SSBYTES],
                   const uint8_t ct[KYBER_CIPHERTEXTBYTES],
                   const uint8_t sk[KYBER_SECRETKEYBYTES]) {
  uint8_t buf[2 * KYBER_SYMBYTES];
  uint8_t kr[2 * KYBER_SYMBYTES];
  uint8_t cmp[KYBER_CIPHERTEXTBYTES];
  const uint8_t *pk = sk + KYBER_POLYVECBYTES;
  const uint8_t *h_pk = sk + KYBER_SECRETKEYBYTES - 2 * KYBER_SYMBYTES;
  const uint8_t *z = sk + KYBER_SECRETKEYBYTES - KYBER_SYMBYTES;
  uint8_t fail;

  // Decrypt to get m'
  indcpa_dec(buf, ct, sk);

  // Compute (K_bar', r') = G(m' || H(pk))
  memcpy(buf + KYBER_SYMBYTES, h_pk, KYBER_SYMBYTES);
  sha3_512(kr, buf, 2 * KYBER_SYMBYTES);

  // Re-encrypt to get c'
  indcpa_enc(cmp, buf, pk, kr + KYBER_SYMBYTES);

  // Compare c and c' in constant time
  fail = 0;
  for (size_t i = 0; i < KYBER_CIPHERTEXTBYTES; i++)
    fail |= ct[i] ^ cmp[i];

  // Compute H(c)
  sha3_256(kr + KYBER_SYMBYTES, ct, KYBER_CIPHERTEXTBYTES);

  // If fail, compute garbage key from z
  // This is implicit rejection - always output something
  uint8_t garbage[2 * KYBER_SYMBYTES];
  memcpy(garbage, z, KYBER_SYMBYTES);
  memcpy(garbage + KYBER_SYMBYTES, kr + KYBER_SYMBYTES, KYBER_SYMBYTES);

  // Constant-time selection: if fail != 0, use garbage instead of kr
  // fail should be 0 or non-zero; convert to 0 or 1
  fail = (fail | (-fail)) >> 7; // 0 if equal, 1 if different

  // Select between real key and garbage in constant time
  select_bytes(kr, garbage, kr, 2 * KYBER_SYMBYTES, (uint8_t)(1 - fail));

  // Derive shared secret
  shake256(ss, KYBER_SSBYTES, kr, 2 * KYBER_SYMBYTES);

  return 0;
}
