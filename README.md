# MNIST NPU Demo for Alif E8 (RTT Output)

MNIST digit recognition on **Alif Ensemble E8** with **Ethos-U55 NPU** acceleration.

Uses **SEGGER RTT** for output - works perfectly on **macOS**!

## 🚀 Quick Start (macOS)

### 1. Build the Firmware

```bash
# Start Docker
docker run -it --rm -v $(pwd):/workspace ubuntu:24.04

# Inside Docker
cd /workspace
./setup.sh
source venv/bin/activate

# Train and build
cd scripts
python train_mnist.py
./run_vela.sh
python generate_headers.py
cd ..
make all
```

### 2. Flash Using SETOOLS

On your Mac (not in Docker), use Alif's SETOOLS:

```bash
cd ~/app-release-exec  # or wherever you installed SETOOLS

# Copy your binary
cp /path/to/mnist_npu_demo.bin build/images/

# Create config (see below) or use existing
app-gen-toc -f build/config/mnist-demo.json
app-write-mram -e app
app-write-mram
```

### 3. View Output via RTT

**Terminal 1** - Connect J-Link:
```bash
JLinkExe -device Cortex-M55 -if SWD -speed 4000 -autoconnect 1
```

**Terminal 2** - Open RTT Viewer:
```bash
JLinkRTTClient
```

Or use the GUI:
```bash
JLinkRTTViewer
```

Then reset the board in Terminal 1:
```
> r
> g
```

## 📺 Expected Output

```
========================================
     ALIF E8 MNIST NPU DEMO
     Ethos-U55 Accelerated
     (RTT Output)
========================================

Initializing NPU... OK

Running inference on test image...
Expected digit: 7

+--------------------------------------+
|       DIGIT RECOGNITION RESULT       |
+--------------------------------------+
  Predicted Digit: 7

        #####
            #
           # 
          #  
         #   

  Confidence: [################----] 85%
  Inference Time: 31 us
  Throughput: 32258 FPS
+--------------------------------------+

>>> CORRECT! <<<

Commands (type in RTT Viewer):
  1 - Run single inference
  2 - Run benchmark (100 iterations)
  3 - Run benchmark (1000 iterations)
  4 - Show model info
  5 - Show output scores
  h - Show this menu

> 
```

## SETOOLS ATOC Configuration

Create `build/config/mnist-demo.json`:

```json
{
    "ATOC": {
        "binary": [
            {
                "name": "MNIST_NPU",
                "binary_path": "images/mnist_npu_demo.bin",
                "load_address": "0x80008000",
                "flags": "0x00000001",
                "cpu_id": "M55_HE",
                "type": "MRAM"
            }
        ]
    }
}
```

## Why RTT Instead of UART?

On Alif E8 DevKit:
- The USB-C exposes the **SE-UART** (Secure Enclave UART) at 57600/100000 baud
- Application UARTs are on **expansion header pins** (not USB)
- **RTT** goes through J-Link debug connection - no extra wiring needed!

## Troubleshooting

### RTT shows nothing

1. Make sure J-Link is connected first
2. Reset the board after connecting RTT: type `r` then `g` in JLinkExe
3. Try `JLinkRTTViewer` (GUI) instead of `JLinkRTTClient`

### J-Link can't connect

```bash
# Try lower speed
JLinkExe -device Cortex-M55 -if SWD -speed 1000
```

### Build errors

```bash
# Make sure ARM toolchain is installed
brew install --cask gcc-arm-embedded
# Or in Docker, it's already there
```

## Files

```
alif-mnist-npu-demo-rtt/
├── app/
│   ├── main.c           # Main app (RTT output)
│   ├── SEGGER_RTT.c/h   # RTT implementation
│   └── npu_driver.c/h   # NPU driver
├── scripts/
│   ├── train_mnist.py   # Training script
│   ├── run_vela.sh      # NPU optimization
│   └── generate_headers.py
├── include/             # Generated headers
├── model/               # Generated models
├── Makefile
├── linker.ld
└── setup.sh
```

## License

MIT License
# alifEthosNPU_Demo
