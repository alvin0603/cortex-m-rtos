CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc
LD      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size
# ---- Project Structure ----
SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build
TARGET    = $(BUILD_DIR)/mini-rtos
# ---- Source Files ----
C_SOURCES = $(shell find $(SRC_DIR) -name '*.c')
ASM_SOURCES = $(shell find $(SRC_DIR) -name '*.s')

# ---- Object Files ----
C_OBJECTS   = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
ASM_OBJECTS = $(patsubst %.s, $(BUILD_DIR)/%.o, $(ASM_SOURCES))
OBJECTS     = $(C_OBJECTS) $(ASM_OBJECTS)

# ---- Compiler Flags ----
CFLAGS  = -mcpu=cortex-m3        # Target CPU
CFLAGS += -mthumb                # Use Thumb instruction set (required for Cortex-M)
CFLAGS += -nostdlib              # Don't link standard C library (we're bare-metal)
CFLAGS += -ffreestanding         # Tell compiler there's no OS
CFLAGS += -Wall -Wextra          # Enable useful warnings
CFLAGS += -O0                    # No optimization (easier to debug)
CFLAGS += -g3                    # Maximum debug info (for GDB)
CFLAGS += -I$(INC_DIR)           # Header search path

# ---- Assembler Flags ----
ASFLAGS  = -mcpu=cortex-m3
ASFLAGS += -mthumb
ASFLAGS += -g3

# ---- Linker Flags ----
LDFLAGS  = -mcpu=cortex-m3
LDFLAGS += -mthumb
LDFLAGS += -nostdlib             # No standard library
LDFLAGS += -T linker.ld          # Use our linker script
LDFLAGS += -Wl,-Map=$(TARGET).map  # Generate a memory map file (useful for debugging)

# ---- QEMU Settings ----
QEMU      = qemu-system-arm
QEMU_MACHINE = lm3s6965evb
QEMU_FLAGS = -machine $(QEMU_MACHINE) -nographic

# Default target: build everything
all: $(TARGET).elf $(TARGET).bin
	@echo ""
	@echo "Build complete!"
	@$(SIZE) $(TARGET).elf

# Link all object files into the final ELF executable
$(TARGET).elf: $(OBJECTS)
	@echo "[LD]  $@"
	@$(LD) $(LDFLAGS) -o $@ $^

# Convert ELF to raw binary (this is what you'd flash onto real hardware)
$(TARGET).bin: $(TARGET).elf
	@echo "[BIN] $@"
	@$(OBJCOPY) -O binary $< $@

# Compile C source files → object files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "[CC]  $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

# Assemble ASM source files → object files
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "[AS]  $<"
	@$(AS) $(ASFLAGS) -c -o $@ $<

# Run on QEMU
run: all
	@echo ""
	@echo "Starting QEMU... (press Ctrl-A then X to exit)"
	@echo "-------------------------------------------"
	$(QEMU) $(QEMU_FLAGS) -kernel $(TARGET).elf

# Run on QEMU with GDB server for debugging
debug: all
	@echo ""
	@echo "Starting QEMU with GDB server on port 3333..."
	@echo "In another terminal, run:"
	@echo "  gdb-multiarch $(TARGET).elf -ex 'target remote :3333'"
	@echo "-------------------------------------------"
	$(QEMU) $(QEMU_FLAGS) -kernel $(TARGET).elf -S -gdb tcp::3333

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run debug clean
