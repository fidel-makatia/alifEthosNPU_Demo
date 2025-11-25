#===============================================================================
# Makefile for MNIST NPU Demo on Alif Ensemble E8
# With SEGGER RTT Output (works on macOS!)
#===============================================================================

TARGET = mnist_npu_demo
BUILD_DIR = build

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

# Sources - includes RTT
C_SOURCES = \
    app/main.c \
    app/npu_driver.c \
    app/SEGGER_RTT.c

C_INCLUDES = -Iinclude -Iapp

# CPU/FPU flags for Cortex-M55
MCU = -mcpu=cortex-m55 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

# Compiler flags
CFLAGS = $(MCU) $(C_INCLUDES)
CFLAGS += -Wall -Wextra
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -fno-common -fno-builtin
CFLAGS += -O2 -g3 -gdwarf-2
CFLAGS += -DALIF_E8 -DETHOS_U55

# Linker flags
LDSCRIPT = linker.ld
LDFLAGS = $(MCU) -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref
LDFLAGS += -Wl,--print-memory-usage
LDFLAGS += -specs=nano.specs -specs=nosys.specs
LDFLAGS += -nostartfiles -lm -lc -lgcc

# Object files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

#-------------------------------------------------------------------------------
# Build Rules
#-------------------------------------------------------------------------------

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	@echo ""
	@echo "=========================================="
	@echo " Build Complete!"
	@echo "=========================================="
	@echo " Binary: $(BUILD_DIR)/$(TARGET).bin"
	@echo ""
	@echo " To view output on macOS:"
	@echo "   1. JLinkExe -device Cortex-M55 -if SWD -speed 4000"
	@echo "   2. In another terminal: JLinkRTTClient"
	@echo "=========================================="

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "CC    $<"
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile $(LDSCRIPT)
	@echo "LD    $@"
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo ""
	@$(SZ) $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	@echo "HEX   $@"
	@$(CP) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo "BIN   $@"
	@$(CP) -O binary -S $< $@

clean:
	@echo "Cleaning..."
	rm -rf $(BUILD_DIR)

size: $(BUILD_DIR)/$(TARGET).elf
	@$(SZ) --format=berkeley $<

.PHONY: all clean size
