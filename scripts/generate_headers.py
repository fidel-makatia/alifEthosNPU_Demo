#!/usr/bin/env python3
"""
Generate C Headers for Embedded Deployment
==========================================
Converts Vela-optimized TFLite model and test data to C headers.
"""

import os
import numpy as np
import json
from datetime import datetime

print("=" * 60)
print("Generating C Headers for Embedded Deployment")
print("=" * 60)

# Paths
MODEL_PATH = "../model/vela_output/mnist_model_vela.tflite"
FALLBACK_MODEL_PATH = "../model/mnist_model.tflite"
TEST_IMAGE_PATH = "../model/test_image_int8.npy"
TEST_LABEL_PATH = "../model/test_label.npy"
QUANT_PARAMS_PATH = "../model/quantization_params.json"
OUTPUT_DIR = "../include"

os.makedirs(OUTPUT_DIR, exist_ok=True)

# Step 1: Read model
print("\nStep 1: Reading model...")
if os.path.exists(MODEL_PATH):
    model_path = MODEL_PATH
else:
    model_path = FALLBACK_MODEL_PATH
    print(f"  Note: Using non-Vela model")

with open(model_path, "rb") as f:
    model_data = f.read()
print(f"  Model: {model_path}")
print(f"  Size: {len(model_data):,} bytes")

# Step 2: Read test data
print("\nStep 2: Reading test data...")
test_image = np.load(TEST_IMAGE_PATH).flatten()
test_label = int(np.load(TEST_LABEL_PATH))
print(f"  Test digit: {test_label}")

# Step 3: Read quantization parameters
print("\nStep 3: Reading quantization parameters...")
if os.path.exists(QUANT_PARAMS_PATH):
    with open(QUANT_PARAMS_PATH, 'r') as f:
        quant_params = json.load(f)
else:
    quant_params = {'input_scale': 0.003921568859, 'input_zero_point': -128,
                    'output_scale': 1.0, 'output_zero_point': 0}

# Step 4: Generate model header
print("\nStep 4: Generating mnist_model_data.h...")
with open(f"{OUTPUT_DIR}/mnist_model_data.h", "w") as f:
    f.write(f"""/**
 * @file mnist_model_data.h
 * @brief MNIST Model Data for Ethos-U55 NPU
 * Auto-generated on {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
 */

#ifndef MNIST_MODEL_DATA_H
#define MNIST_MODEL_DATA_H

#include <stdint.h>
#include <stddef.h>

#define MNIST_MODEL_SIZE {len(model_data)}

__attribute__((aligned(16), section(".rodata")))
const uint8_t mnist_model_data[MNIST_MODEL_SIZE] = {{
""")
    for i in range(0, len(model_data), 16):
        chunk = model_data[i:i+16]
        hex_vals = ", ".join(f"0x{b:02X}" for b in chunk)
        f.write(f"    {hex_vals}{',' if i+16 < len(model_data) else ''}\n")
    f.write("""};

#endif /* MNIST_MODEL_DATA_H */
""")

# Step 5: Generate test data header
print("Step 5: Generating test_data.h...")
with open(f"{OUTPUT_DIR}/test_data.h", "w") as f:
    f.write(f"""/**
 * @file test_data.h
 * @brief Test Data - Expected digit: {test_label}
 * Auto-generated on {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
 */

#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <stdint.h>

#define TEST_IMAGE_SIZE    784
#define NUM_CLASSES        10
#define EXPECTED_DIGIT     {test_label}

__attribute__((aligned(16)))
const int8_t test_input_data[TEST_IMAGE_SIZE] = {{
""")
    for i in range(0, len(test_image), 28):
        chunk = test_image[i:i+28]
        vals = ", ".join(f"{int(v):4d}" for v in chunk)
        f.write(f"    {vals}{',' if i+28 < len(test_image) else ''}\n")
    f.write("""};

#endif /* TEST_DATA_H */
""")

# Step 6: Generate config header
print("Step 6: Generating model_config.h...")
with open(f"{OUTPUT_DIR}/model_config.h", "w") as f:
    f.write(f"""/**
 * @file model_config.h
 * @brief Model Configuration
 * Auto-generated on {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
 */

#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <stdint.h>

/* Model dimensions */
#define MODEL_INPUT_WIDTH      28
#define MODEL_INPUT_HEIGHT     28
#define MODEL_INPUT_CHANNELS   1
#define MODEL_INPUT_SIZE       784
#define MODEL_OUTPUT_SIZE      10
#define MODEL_NUM_CLASSES      10

/* Quantization parameters */
#define INPUT_SCALE            {quant_params['input_scale']:.10f}f
#define INPUT_ZERO_POINT       {quant_params['input_zero_point']}
#define OUTPUT_SCALE           {quant_params['output_scale']:.10f}f
#define OUTPUT_ZERO_POINT      {quant_params['output_zero_point']}

/* Memory configuration */
#define TENSOR_ARENA_SIZE      (128 * 1024)

/* Hardware addresses */
#define ETHOS_U_BASE_ADDR      0x50004000UL
#define SYSTEM_CLOCK_HZ        160000000UL

#endif /* MODEL_CONFIG_H */
""")

print("\n" + "=" * 60)
print("HEADER GENERATION COMPLETE")
print("=" * 60)
print(f"\nGenerated files in {OUTPUT_DIR}/:")
print(f"  - mnist_model_data.h ({len(model_data):,} bytes)")
print(f"  - test_data.h (digit {test_label})")
print(f"  - model_config.h")
print("\nNext: cd .. && make all")
print("=" * 60)
