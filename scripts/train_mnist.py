#!/usr/bin/env python3
"""
MNIST CNN Training for Ethos-U55 NPU Deployment
================================================
Compatible with TensorFlow 2.16+ and Keras 3

This script:
1. Trains a CNN on MNIST dataset
2. Quantizes to INT8 for embedded deployment
3. Exports TFLite model ready for Vela optimization
"""

import tensorflow as tf
import numpy as np
import os
import tempfile
import json

print("=" * 70)
print("MNIST Training for Alif E8 Ethos-U55 NPU")
print("=" * 70)
print(f"TensorFlow version: {tf.__version__}")
print()

# Set seeds for reproducibility
np.random.seed(42)
tf.random.set_seed(42)

###############################################################################
# STEP 1: Load MNIST Dataset
###############################################################################
print("\n" + "=" * 70)
print("STEP 1: Loading MNIST Dataset")
print("=" * 70)

(x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()

# Normalize to [0, 1] range
x_train = x_train.astype("float32") / 255.0
x_test = x_test.astype("float32") / 255.0

# Add channel dimension: (N, 28, 28) -> (N, 28, 28, 1)
x_train = np.expand_dims(x_train, axis=-1)
x_test = np.expand_dims(x_test, axis=-1)

print(f"Training samples:   {x_train.shape[0]:,}")
print(f"Test samples:       {x_test.shape[0]:,}")
print(f"Image shape:        {x_train.shape[1:]}")

###############################################################################
# STEP 2: Build CNN Model (Optimized for Ethos-U55 NPU)
###############################################################################
print("\n" + "=" * 70)
print("STEP 2: Building CNN Model")
print("=" * 70)

model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(28, 28, 1), name="input"),
    
    # Conv Block 1 - NPU accelerated
    tf.keras.layers.Conv2D(8, 3, padding='same', name="conv1"),
    tf.keras.layers.BatchNormalization(name="bn1"),
    tf.keras.layers.ReLU(name="relu1"),
    tf.keras.layers.MaxPooling2D(2, name="pool1"),
    
    # Conv Block 2 - NPU accelerated
    tf.keras.layers.Conv2D(16, 3, padding='same', name="conv2"),
    tf.keras.layers.BatchNormalization(name="bn2"),
    tf.keras.layers.ReLU(name="relu2"),
    tf.keras.layers.MaxPooling2D(2, name="pool2"),
    
    # Classification Head
    tf.keras.layers.Flatten(name="flatten"),
    tf.keras.layers.Dense(10, name="output")
], name="mnist_cnn")

model.summary()

total_params = model.count_params()
print(f"\nTotal parameters: {total_params:,}")

###############################################################################
# STEP 3: Compile and Train Model
###############################################################################
print("\n" + "=" * 70)
print("STEP 3: Training Model")
print("=" * 70)

model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
    loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
    metrics=['accuracy']
)

history = model.fit(
    x_train, y_train,
    batch_size=128,
    epochs=10,
    validation_split=0.1,
    verbose=1
)

###############################################################################
# STEP 4: Evaluate Model
###############################################################################
print("\n" + "=" * 70)
print("STEP 4: Evaluating Model")
print("=" * 70)

test_loss, test_accuracy = model.evaluate(x_test, y_test, verbose=0)
print(f"Test Accuracy: {test_accuracy:.4f} ({test_accuracy*100:.2f}%)")

###############################################################################
# STEP 5: Representative Dataset for Quantization
###############################################################################
print("\n" + "=" * 70)
print("STEP 5: Preparing Quantization")
print("=" * 70)

def representative_dataset_generator():
    """Generate samples for INT8 quantization calibration."""
    indices = np.random.choice(len(x_train), 200, replace=False)
    for idx in indices:
        yield [x_train[idx:idx+1].astype(np.float32)]

###############################################################################
# STEP 6: Convert to TFLite INT8 (Keras 3 Compatible)
###############################################################################
print("\n" + "=" * 70)
print("STEP 6: Converting to TFLite INT8")
print("=" * 70)

os.makedirs("../model", exist_ok=True)

# FIX FOR KERAS 3: Export to SavedModel first
print("Exporting to SavedModel format...")
with tempfile.TemporaryDirectory() as saved_model_dir:
    model.export(saved_model_dir)
    
    print("Converting to TFLite INT8...")
    converter = tf.lite.TFLiteConverter.from_saved_model(saved_model_dir)
    
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset_generator
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    
    tflite_model = converter.convert()

# Save model
tflite_path = "../model/mnist_model.tflite"
with open(tflite_path, "wb") as f:
    f.write(tflite_model)

print(f"Model saved: {tflite_path}")
print(f"Model size: {len(tflite_model)/1024:.1f} KB")

###############################################################################
# STEP 7: Verify Quantized Model
###############################################################################
print("\n" + "=" * 70)
print("STEP 7: Verifying Quantized Model")
print("=" * 70)

interpreter = tf.lite.Interpreter(model_content=tflite_model)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()[0]
output_details = interpreter.get_output_details()[0]

print(f"Input: {input_details['dtype']}, shape {input_details['shape']}")
print(f"Output: {output_details['dtype']}, shape {output_details['shape']}")

input_scale = input_details['quantization'][0]
input_zero_point = input_details['quantization'][1]

# Test accuracy
correct = 0
for i in range(1000):
    test_image = x_test[i:i+1]
    if input_scale != 0:
        test_image_int8 = (test_image / input_scale + input_zero_point).astype(np.int8)
    else:
        test_image_int8 = ((test_image * 255) - 128).astype(np.int8)
    
    interpreter.set_tensor(input_details['index'], test_image_int8)
    interpreter.invoke()
    output = interpreter.get_tensor(output_details['index'])[0]
    
    if np.argmax(output) == y_test[i]:
        correct += 1

quantized_accuracy = correct / 1000
print(f"Quantized Accuracy: {quantized_accuracy:.4f}")

###############################################################################
# STEP 8: Save Test Data
###############################################################################
print("\n" + "=" * 70)
print("STEP 8: Saving Test Data")
print("=" * 70)

test_idx = 0
test_image = x_test[test_idx]
test_label = int(y_test[test_idx])

if input_scale != 0:
    test_image_int8 = (test_image / input_scale + input_zero_point).astype(np.int8)
else:
    test_image_int8 = ((test_image * 255) - 128).astype(np.int8)

np.save("../model/test_image_int8.npy", test_image_int8)
np.save("../model/test_label.npy", test_label)

quant_params = {
    'input_scale': float(input_scale),
    'input_zero_point': int(input_zero_point),
    'output_scale': float(output_details['quantization'][0]),
    'output_zero_point': int(output_details['quantization'][1])
}
with open("../model/quantization_params.json", "w") as f:
    json.dump(quant_params, f, indent=2)

print(f"Test image saved (digit: {test_label})")

###############################################################################
# SUMMARY
###############################################################################
print("\n" + "=" * 70)
print("TRAINING COMPLETE")
print("=" * 70)
print(f"  Float32 Accuracy:    {test_accuracy:.4f}")
print(f"  INT8 Accuracy:       {quantized_accuracy:.4f}")
print(f"  Model Size:          {len(tflite_model)/1024:.1f} KB")
print(f"  Test Digit:          {test_label}")
print("=" * 70)
print("\nNext: Run ./run_vela.sh to optimize for NPU")
print("=" * 70)
