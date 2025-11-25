#!/bin/bash
#===============================================================================
# Setup Script for MNIST NPU Demo
# Installs all dependencies in Docker container
#===============================================================================

set -e

echo "========================================"
echo " MNIST NPU Demo - Environment Setup"
echo "========================================"
echo ""

# Check if running in Docker
if [ ! -f /.dockerenv ]; then
    echo "WARNING: This script is designed to run inside a Docker container."
    echo "Continue anyway? (y/n)"
    read -r response
    if [ "$response" != "y" ]; then
        exit 1
    fi
fi

# Update system
echo "[1/5] Updating system packages..."
apt-get update && apt-get upgrade -y

# Install build tools
echo "[2/5] Installing build tools..."
apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    nano \
    xxd \
    python3 \
    python3-pip \
    python3-venv

# Install ARM toolchain
echo "[3/5] Installing ARM toolchain..."
apt-get install -y gcc-arm-none-eabi

# Verify ARM toolchain
echo "    ARM GCC version:"
arm-none-eabi-gcc --version | head -1

# Setup Python environment
echo "[4/5] Setting up Python environment..."
cd /workspace

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

source venv/bin/activate

pip install --upgrade pip
pip install tensorflow==2.16.1
pip install ethos-u-vela==3.11.0
pip install numpy matplotlib pillow

# Verify installations
echo "[5/5] Verifying installations..."
echo "    TensorFlow version: $(python3 -c 'import tensorflow as tf; print(tf.__version__)')"
echo "    Vela version: $(vela --version)"

# Create necessary directories
mkdir -p model/vela_output
mkdir -p include
mkdir -p build

echo ""
echo "========================================"
echo " Setup Complete!"
echo "========================================"
echo ""
echo " Next steps:"
echo "   1. source venv/bin/activate"
echo "   2. cd scripts && python train_mnist.py"
echo "   3. ./run_vela.sh"
echo "   4. python generate_headers.py"
echo "   5. cd .. && make all"
echo ""
