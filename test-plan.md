# Kyber C-Port Test Plan (TDD)

This document outlines the testing strategy for the Kyber (ML-KEM) C implementation using the **Unity** framework.

## 1. Unit Testing Strategy
Every component will follow the **Red-Green-Refactor** loop. Tests will be located in `test/test_*.c`.

### Phase 1: Symmetric & Utility Functions
| Component | Test Case | Purpose | Reference |
| :--- | :--- | :--- | :--- |
| `utils.c` | `select_bytes` | Constant-time selection | Python `utils.py` |
| `fips202.c` | `sha3_256` | Hash correctness | NIST FIPS 202 (256-bit) |
| `fips202.c` | `sha3_512` | Hash correctness | NIST FIPS 202 (512-bit) |
| `fips202.c` | `shake128` | XOF correctness | NIST FIPS 202 (XOF) |
| `fips202.c` | `shake256` | XOF correctness | NIST FIPS 202 (XOF) |

### Phase 2: Polynomial Arithmetic (The Core)
| Component | Test Case | Reference Source |
| :--- | :--- | :--- |
| `poly.c` | `cbd` (Centered Binomial Distribution) | Python `polynomials.py` |
| `poly.c` | `ntt` / `invntt` (Transform parity) | Identity: $x = \text{InvNTT}(\text{NTT}(x))$ |
| `poly.c` | `basemul` (Multiplication in NTT domain) | Python `polynomials.py` |
| `poly.c` | `compress` / `decompress` (Lossy check) | Python `polynomials.py` |

### Phase 3: Module Operations
| Component | Test Case | Reference Source |
| :--- | :--- | :--- |
| `module.c` | Matrix-Vector Multiplication | Python `modules.py` |
| `module.c` | Vector-Vector dot product | Python `modules.py` |

### Phase 4: Integration (PKE & KEM)
| Component | Test Case | Reference Source |
| :--- | :--- | :--- |
| `kem.c` | Key Generation Consistency | Fixed seed comparison with Python |
| `kem.c` | Encapsulation / Decapsulation Loop | $K == \text{Decaps}(sk, \text{Encaps}(pk))$ |
| `kem.c` | FIPS 203 KAT | Official NIST Known Answer Tests |

## 2. Testing Tools
*   **Framework**: [Unity](https://github.com/ThrowTheSwitch/Unity)
*   **Runner**: `test.ps1` (PowerShell script to automate compilation and execution).
*   **Comparison**: Using the existing `python-kyber-py` codebase to generate intermediate expected values for C unit tests.

## 3. How to add a new test
1.  Create `test/test_<filename>.c`.
2.  Include `unity.h` and the relevant header from `include/`.
3.  Add the test to the `main()` function in the test file using `RUN_TEST()`.
4.  Execute via `powershell -ExecutionPolicy Bypass -File test.ps1`.
