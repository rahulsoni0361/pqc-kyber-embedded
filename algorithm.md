# Kyber (ML-KEM) Encryption Algorithm Under the Hood

This document outlines the step-by-step mathematical and logical process of the Kyber encryption (CPAPKE.Enc) as we port it from Python to C.

## 1. Inputs
*   **Public Key ($pk$):** A byte array containing the serialized vector $\hat{t}$ and the seed $\rho$.
*   **Message ($m$):** 32 bytes (256 bits) of data to be encrypted.
*   **Randomness ($r$):** 32 bytes used to seed the error sampling (ensures the scheme is probabilistic).

## 2. Step-by-Step Execution

### Step A: Unpacking the Public Key
The public key is split into:
1.  **$\hat{t}$**: The polynomial vector in the NTT domain.
2.  **$\rho$**: The seed used to re-generate the matrix $A$.

### Step B: Building the Matrix $A$
Using the seed $\rho$ and an Extendable Output Function (XOF) like SHAKE-128, we generate a $k \times k$ matrix of polynomials. For encryption, we use the transpose $A^T$.
*   **Goal**: Create $A[i][j]$ for all $i, j \in \{0, \dots, k-1\}$.

### Step C: Sampling Errors (The "Noise")
We sample three components from a Centered Binomial Distribution (CBD):
1.  **$r$**: A secret vector (simulated "short" noise).
2.  **$e_1$**: An error vector added to the first part of the ciphertext.
3.  **$e_2$**: An error polynomial added to the second part of the ciphertext.

### Step D: NTT Transformation
The sampled vector $r$ is transformed into the NTT domain ($\hat{r}$) to allow for fast polynomial multiplication with the matrix $A^T$.

### Step E: Core Arithmetic (In the Lattice)
1.  **Compute $u$**: 
    $$u = \text{InvNTT}(A^T \circ \hat{r}) + e_1$$
    *(This hides the secret $r$ within the lattice structure)*
2.  **Compute $v$**:
    $$v = \text{InvNTT}(\hat{t}^T \circ \hat{r}) + e_2 + \text{MessagePoly}$$
    *(The message is embedded here by shifting its bits)*

### Step F: Compression & Serialization
To reduce the size of the ciphertext, we:
1.  **Compress**: Map the large coefficients (mod 3329) to a smaller bit-range ($d_u$ and $d_v$).
2.  **Encode**: Convert the compressed polynomials into a byte array.

---

## Technical Challenges for C Implementation
*   **Memory alignment**: Handling arrays of `int16_t` efficiently.
*   **NTT efficiency**: Writing the butterfly operations without Python's high-level overhead.
*   **Constant time**: Ensuring that branching doesn't leak info about the secret $r$ (though $r$ is ephemeral here).
