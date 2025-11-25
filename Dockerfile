# Dockerfile for MNIST NPU Demo Development
# Build: docker build -t alif-mnist-dev .
# Run:   docker run -it --rm -v $(pwd):/workspace alif-mnist-dev

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    nano \
    xxd \
    python3 \
    python3-pip \
    python3-venv \
    gcc-arm-none-eabi \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Create and activate virtual environment, install Python dependencies
RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

RUN pip install --upgrade pip && \
    pip install tensorflow==2.16.1 ethos-u-vela==3.11.0 numpy matplotlib pillow

# Verify installations
RUN arm-none-eabi-gcc --version && \
    python3 -c "import tensorflow as tf; print(f'TensorFlow: {tf.__version__}')" && \
    vela --version

# Default command
CMD ["/bin/bash"]
