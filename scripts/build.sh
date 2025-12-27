#!/bin/bash
set -e

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
# Project root is one level up from scripts
PROJECT_ROOT="$SCRIPT_DIR/.."

# Create build directory inside the project root
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure (pointing to PROJECT_ROOT)
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5

# Build (utilizing all cores)
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "Build complete."
