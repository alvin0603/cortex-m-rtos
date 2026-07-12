CROSS_COMPILE ?= arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE    = $(CROSS_COMPILE)size

SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build
TARGET    = $(BUILD_DIR)/mini-rtos

C_SRCS   = $(shell find $(SRC_DIR) -name '*.c')
ASM_SRCS = $(shell find $(SRC_DIR) -name '*.s')

OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS)) \
       $(patsubst %.s, $(BUILD_DIR)/%.o, $(ASM_SRCS))

ARCH_FLAGS = -mcpu=cortex-m3 -mthumb
CFLAGS     = $(ARCH_FLAGS) -nostdlib -ffreestanding -Wall -Wextra -O2 -g3 -I$(INC_DIR)
ASFLAGS    = $(ARCH_FLAGS) -g3
LDFLAGS    = $(ARCH_FLAGS) -nostdlib -T linker.ld -Wl,-Map=$(TARGET).map

QEMU       = qemu-system-arm
QEMU_FLAGS = -machine lm3s6965evb -nographic -icount shift=0,align=off

.PHONY: all clean run debug

all: $(TARGET).bin $(TARGET).elf
	@$(SIZE) $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c -o $@ $<

run: all
	$(QEMU) $(QEMU_FLAGS) -kernel $(TARGET).elf

debug: all
	$(QEMU) $(QEMU_FLAGS) -kernel $(TARGET).elf -S -gdb tcp::3333

clean:
	rm -rf $(BUILD_DIR)
