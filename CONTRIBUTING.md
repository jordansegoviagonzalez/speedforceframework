# Contributing to SpeedForce

We welcome contributions to the SpeedForce project! This document outlines guidelines for contributing, including code style, testing, and pull request procedures.

## How to Contribute

*   **Bug Reports:** If you find a bug, please open an issue on our GitHub repository.
*   **Feature Requests:** We'd love to hear your ideas for new features. Open an issue to discuss your proposal.
*   **Code Contributions:**
    1.  Fork the repository.
    2.  Create a new branch for your feature or bug fix.
    3.  Implement your changes, ensuring they adhere to our code style and include appropriate tests.
    4.  Submit a pull request.

## Code Style

*   **C++:** We use `clang-format`. Please run `./scripts/format.sh` before submitting a pull request.
*   **Python:** We use `black` and `ruff`. Please run `./scripts/format.sh` before submitting a pull request.

## Testing

*   All new features and bug fixes should be accompanied by unit and/or integration tests.
*   Run C++ tests with `ctest` after building the project.
*   Run Python tests with `pytest python/tests`.

## Pull Request Guidelines

*   Ensure your code compiles and all tests pass.
*   Provide a clear and concise description of your changes.
*   Reference any related issues.

## License

By contributing to SpeedForce, you agree that your contributions will be licensed under the project's MIT license.

Thank you for contributing to SpeedForce!
