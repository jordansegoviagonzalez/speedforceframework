# SpeedForce Architecture

This document provides a detailed look into the technical architecture of the SpeedForce engine and serving framework. It is intended for core developers and contributors.

## 1. Core Principles

The design of SpeedForce is guided by four primary principles:

*   **Performance:** The engine's core will be written in modern C++ to achieve near-the-metal performance. It will leverage the best available vendor-specific libraries (e.g., TensorRT-LLM, ROCm).
*   **Usability:** A clean, intuitive Python API will be the primary user interface. Developers should be able to achieve production-grade performance with just a few lines of Python code.
*   **Modularity & Portability:** A plugin-based architecture will abstract the core engine from the underlying hardware, enabling support for a wide range of devices (NVIDIA, AMD, Intel, ARM) without changing the core logic.
*   **Scalability:** The serving framework will be built on gRPC and will incorporate advanced scheduling techniques (e.g., continuous batching) to handle thousands of concurrent requests efficiently.

## 2. System Components

The system is composed of four primary, decoupled components that interact in layers.

```
+--------------------------------------------------------------------------+
|                                User Layer                                |
|--------------------------------------------------------------------------|
| Python Application / Jupyter Notebook |        Remote Client Application |
+---------------------------------------+----------------------------------+
              | (Python API)                          | (gRPC)
              |                                       |
+---------------------------------------+----------------------------------+
|      `speedforce` (Python Package)    |  `speedforce-server` (gRPC Server)|
| (User-friendly Python wrappers)       |  (Request Queuing & Scheduling)  |
+--------------------------------------------------------------------------+
              |                                       |
              | (pybind11 C++ Bindings)               | (Direct C++ Call)
              |                                       |
+--------------------------------------------------------------------------+
|                          `libspeedforce` (C++ Core Engine)                 |
|--------------------------------------------------------------------------|
| Tensor Management | Model Loading | KV Cache Management | Request Scheduler|
+--------------------------------------------------------------------------+
                                      | (Backend Plugin Interface)
                                      |
+--------------------------+----------------------------+-------------------+
|  TensorRT-LLM Backend    |   ONNX Runtime Backend     |    ROCm Backend   |
|      (for NVIDIA)        | (for CPU, AMD, Cross-Vendor)|    (for AMD)      |
+--------------------------+----------------------------+-------------------+
              |                        |                        |
+--------------------------+----------------------------+-------------------+
|           NVIDIA GPU     |            CPU             |      AMD GPU      |
+--------------------------------------------------------------------------+
|                               Hardware Layer                             |
+--------------------------------------------------------------------------+
```

### Component Deep-Dive

*   **User Layer:** The end-user application. This can be a Python script, a Jupyter notebook, or a remote application making requests to the server. The user interacts with SpeedForce through a high-level, language-specific API.

*   **Language Bindings & Serving Framework:**
    *   **Python Package (`speedforce`):** Provides the simple, user-friendly Python API. It uses `pybind11` to wrap the C++ core engine, making powerful C++ functionality available idiomatically in Python.
    *   **gRPC Server (`speedforce-server`):** A standalone C++ application that hosts `libspeedforce` and exposes it over the network via a gRPC API. This component is responsible for handling incoming requests, queuing them, and using the continuous batching scheduler to feed them to the engine efficiently.

*   **Core Engine (`libspeedforce`):** This is the heart of SpeedForce, implemented as a shared C++ library.
    *   **Public API (`engine.h`, `model.h`):** Defines the stable C++ interface that the language bindings and gRPC server are built against.
    *   **Internal Logic:** Contains the implementations for model loading, tensor manipulation, and memory management.
    *   **Scheduler (`scheduler.h`):** The implementation of the continuous batching algorithm, which is critical for maximizing throughput in the serving scenario.
    *   **Backend Interface (`backends/backend.h`):** A crucial abstraction layer. This file defines a pure virtual `IBackend` interface class. The core engine only interacts with this abstract interface, making it completely decoupled from any specific hardware or acceleration library.

*   **Backend Plugins:** These are hardware-specific implementations of the `IBackend` interface, compiled as separate shared libraries (`.so`, `.dll`). The core engine loads the appropriate plugin at runtime based on the available hardware and user configuration.
    *   **TensorRT-LLM Backend:** A plugin that uses NVIDIA's TensorRT-LLM library to execute models on NVIDIA GPUs. This will provide the highest possible performance on NVIDIA hardware.
    *   **ONNX Runtime Backend:** A plugin that uses the ONNX Runtime. This is a highly versatile backend that can execute models on CPUs, and can also be used as a cross-vendor GPU backend (including AMD, Intel). It serves as an excellent default/fallback.
    *   **ROCm Backend:** A future plugin to specifically target AMD GPUs using AMD's ROCm software stack.

*   **Hardware Layer:** The physical CPU or GPU that executes the code.

## 3. Project Directory Philosophy

The project is structured as a **monorepo** containing multiple related sub-projects. This is intentional.

```
speedforce/
├── cpp/            # The C++ core engine, server, and backends
├── python/         # The Python user-facing package
├── docs/           # All documentation
├── tools/          # Related command-line tools (e.g., model converter)
└── ...
```

This structure allows for:
*   **Atomic Commits:** Changes that affect both the C++ core and the Python bindings can be made in a single commit, ensuring the project is always in a consistent state.
*   **Unified Build & Test:** A single top-level command can be used to build and test the entire project, simplifying CI/CD.
*   **Clear Separation of Concerns:** While in a monorepo, each component (C++, Python, etc.) is a self-contained unit with its own tests and dependencies, making it easy to understand and maintain.

## 4. Documentation Strategy

To ensure SpeedForce is easy to understand and use, we employ a multi-faceted documentation strategy:

*   **User Guides (`docs/guides/`):** Step-by-step tutorials and how-to guides for getting started with SpeedForce, deploying models, and leveraging its features.
    *   [Getting Started Guide](./guides/getting_started.md)
    *   (Future: Model Deployment Guide, Performance Tuning Guide, etc.)

*   **API Reference (`docs/cpp_api/`, `docs/python_api/`):** Detailed, automatically generated documentation for all public APIs.
    *   **C++ API:** Generated using [Doxygen](https://www.doxygen.nl/index.html) from comments in the C++ header files. (Coming Soon)
    *   **Python API:** Generated using [Sphinx](https://www.sphinx-doc.org/en/master/) from Python docstrings and type hints. (Coming Soon)

*   **Architecture & Design (`docs/architecture.md`):** High-level overviews and deep dives into the system's design principles and component interactions. (This Document)

*   **Contributing Guide (`CONTRIBUTING.md`):** Instructions for community members on how to contribute to the SpeedForce project, including code style, testing guidelines, and pull request procedures. (Coming Soon)