# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-12-02

### Added
- **Core Engine:** Initial C++ implementation of the inference engine (`libspeedforce`).
- **ONNX Backend:** Integrated ONNX Runtime for CPU and cross-vendor GPU inference support.
- **gRPC Server:** High-performance serving layer with `LoadModel` and `Infer` endpoints.
- **Python Bindings:** `speedforce` Python package for easy integration.
- **Docker Support:** Multi-stage `Dockerfile` for enterprise deployment.
- **Build System:** CMake-based build system with automated dependency management (gRPC, Protobuf).

### Security
- **Dependency Management:** Pinned versions for gRPC (v1.62.0) and ONNX Runtime to ensure stability.
