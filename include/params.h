#ifndef PARAMS_H
#define PARAMS_H

#include <stdint.h>

/*************************************************
 * Kyber Parameters
 * Based on FIPS 203 (ML-KEM) specification
 *************************************************/

#define KYBER_N 256  // Polynomial degree
#define KYBER_Q 3329 // Modulus

#define KYBER_SYMBYTES 32 // Size of hashes, seeds
#define KYBER_SSBYTES 32  // Size of shared key

// Kyber-512 parameters (default)
#ifndef KYBER_K
#define KYBER_K 2
#endif

#if (KYBER_K == 2)
#define KYBER_ETA1 3
#define KYBER_ETA2 2
#define KYBER_DU 10
#define KYBER_DV 4
#define KYBER_POLYVECBYTES 768
#define KYBER_POLYCOMPRESSEDBYTES 128
#define KYBER_POLYVECCOMPRESSEDBYTES (KYBER_K * 320)

#elif (KYBER_K == 3)
#define KYBER_ETA1 2
#define KYBER_ETA2 2
#define KYBER_DU 10
#define KYBER_DV 4
#define KYBER_POLYVECBYTES 1152
#define KYBER_POLYCOMPRESSEDBYTES 128
#define KYBER_POLYVECCOMPRESSEDBYTES (KYBER_K * 320)

#elif (KYBER_K == 4)
#define KYBER_ETA1 2
#define KYBER_ETA2 2
#define KYBER_DU 11
#define KYBER_DV 5
#define KYBER_POLYVECBYTES 1536
#define KYBER_POLYCOMPRESSEDBYTES 160
#define KYBER_POLYVECCOMPRESSEDBYTES (KYBER_K * 352)

#else
#error "KYBER_K must be in {2,3,4}"
#endif

// Derived sizes
#define KYBER_PUBLICKEYBYTES (KYBER_POLYVECBYTES + KYBER_SYMBYTES)
#define KYBER_SECRETKEYBYTES                                                   \
  (KYBER_POLYVECBYTES + KYBER_PUBLICKEYBYTES + 2 * KYBER_SYMBYTES)
#define KYBER_CIPHERTEXTBYTES                                                  \
  (KYBER_POLYVECCOMPRESSEDBYTES + KYBER_POLYCOMPRESSEDBYTES)

#endif /* PARAMS_H */
