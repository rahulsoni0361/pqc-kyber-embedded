# Test Driven Development Plan (C)

We are using **Unity** as our unit testing framework. It is lightweight, embedded-friendly, and perfect for testing cryptographic algorithms.

## Project Structure
- `src/`: Implementation files (`.c`).
- `include/`: Header files (`.h`).
- `test/`: Unit tests (`test_*.c`).
- `test/vendor/`: Unity framework files.
- `build/`: Compilation artifacts.

## The TDD Loop
1. **Red**: Define the interface in `include/` and write a failing test in `test/`.
2. **Green**: Implement the function in `src/` to make the test pass.
3. **Refactor**: Optimize (e.g., improve NTT performance) while keeping tests green.

## Running Tests
Run `./test.ps1` to compile and execute all tests.