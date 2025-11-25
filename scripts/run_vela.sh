#!/bin/bash
#===============================================================================
# Run Vela Compiler for Ethos-U55 NPU Optimization
#===============================================================================

set -e

echo "========================================"
echo " Vela NPU Optimization"
echo "========================================"

cd "$(dirname "$0")/.."

# Check if model exists
if [ ! -f "model/mnist_model.tflite" ]; then
    echo "ERROR: model/mnist_model.tflite not found!"
    echo "Run 'python scripts/train_mnist.py' first."
    exit 1
fi

# Create output directory
mkdir -p model/vela_output

# Run Vela compiler
echo ""
echo "Running Vela compiler..."
echo ""

vela model/mnist_model.tflite \
    --accelerator-config ethos-u55-128 \
    --config vela_config.ini \
    --system-config Ethos_U55_High_End_Embedded \
    --memory-mode Shared_Sram \
    --output-dir model/vela_output

echo ""
echo "========================================"
echo " Vela Optimization Complete"
echo "========================================"
echo ""
echo "Original model:  model/mnist_model.tflite"
echo "Optimized model: model/vela_output/mnist_model_vela.tflite"
echo ""

# Show file sizes
echo "File sizes:"
ls -lh model/mnist_model.tflite
ls -lh model/vela_output/mnist_model_vela.tflite

echo ""
echo "Next: Run 'python scripts/generate_headers.py'"
echo ""
