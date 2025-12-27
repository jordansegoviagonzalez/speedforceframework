# --- Build Stage ---
FROM ubuntu:22.04 AS builder

# Install system dependencies
# - cmake, build-essential: for compiling C++
# - git: for fetching dependencies
# - python3-dev: for Python bindings
# - libssl-dev: for gRPC/OpenSSL
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    python3 \
    python3-dev \
    python3-pip \
    libssl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Run the build script
# We ensure the script is executable
RUN chmod +x scripts/build.sh && ./scripts/build.sh

# --- Runtime Stage ---
FROM ubuntu:22.04

# Install runtime dependencies
# - python3: to run the server/bindings
# - libgomp1: for OpenMP support (often needed by ML libraries)
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy built artifacts from builder
COPY --from=builder /app/build/speedforce_server /app/bin/speedforce_server
COPY --from=builder /app/python/src/speedforce /app/speedforce_python/speedforce

# Add Python bindings to path
ENV PYTHONPATH="${PYTHONPATH}:/app/speedforce_python"

# Expose gRPC port
EXPOSE 50051

# Default entrypoint is the server
CMD ["/app/bin/speedforce_server"]
