# Getting Started with SpeedForce

Welcome to the SpeedForce Getting Started Guide! This document provides a step-by-step tutorial for installing SpeedForce, building it from the source, and running your first inference.

> **Note:** This project is in its early stages. The instructions below are a placeholder for what the process will look like in the future.

## 1. Installation

Our goal is to make installation as simple as possible. The primary way to install SpeedForce will be through standard package managers.

### Using `pip` (for Python users)

In the future, you will be able to install the pre-compiled Python package directly from PyPI:

```bash
# This will be enabled once we publish the first version
pip install speedforce
```

This will be the easiest way for most Python developers to get started.

## 2. Building from Source

For contributors or users who need the absolute latest features, building from source will be the primary method.

### Prerequisites

Before you begin, you will need to have the following tools installed on your system:

*   **Git:** To clone the repository.
*   **A C++20 Compiler:**
    *   On Linux: GCC 10+ or Clang 12+
    *   On Windows: Visual Studio 2022 (with the "Desktop development with C++" workload)
*   **CMake (version 3.20 or later):** The build system generator for the C++ core.
*   **Python (version 3.8 or later):** Required for the Python bindings and helper scripts.

For GPU support, you will also need the appropriate development libraries installed:
*   **NVIDIA:** CUDA Toolkit 11.8+ and cuDNN 8+.
*   **AMD:** ROCm 5.0+.

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone --recursive https://github.com/your-repo/speedforce.git
    cd speedforce
    ```
    *(Note: The `--recursive` flag will be used to pull in any git submodules we might use for dependencies.)*

2.  **Run the unified build script:**
    We provide a helper script that handles CMake configuration and building for you.
    ```bash
    ./scripts/build.sh --with-python --with-tensorrt
    ```
    This script will have options to enable or disable different components, such as language bindings or specific hardware backends.

## 3. Your First Inference

Once SpeedForce is installed, you can use the simple Python API to run and accelerate your models.

### Step 3.1: Convert Your Model

SpeedForce uses an optimized internal format. You will first use the `speedforce-convert` tool to prepare your model.

```bash
# This tool will convert a trained PyTorch/JAX/TF model into the SpeedForce format
speedforce-convert \
    --framework jax \
    --model_path /path/to/your/model \
    --output_path /models/my_optimized_model.sf \
    --quantize int8
```

### Step 3.2: Write Your Python Script

Create a new Python file, e.g., `test.py`:

```python
import speedforce
import time

# 1. Load the optimized model into the SpeedForce engine.
# This step loads the model into GPU memory and prepares it for execution.
print("Loading model...")
model = speedforce.load("/models/my_optimized_model.sf")
print("Model loaded successfully.")

# 2. Define your prompt
prompt = "The SpeedForce engine is designed to be"

# 3. Run inference
print(f"Running inference for prompt: '{prompt}'")
start_time = time.time()
result = model.generate(prompt, max_new_tokens=50)
end_time = time.time()

# 4. Print the results
print("\n--- Inference Complete ---")
print(f"Generated text: {result.text}")
print(f"Time taken: {end_time - start_time:.4f} seconds")
```

This example demonstrates the simplicity we are aiming for. The user only needs to learn a handful of commands (`load`, `generate`) to get the full power of the C++ engine.
