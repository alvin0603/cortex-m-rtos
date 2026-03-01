# cortex-m-rtos

A minimal Real-Time Operating System (RTOS) implemented from scratch in C, targeting ARM Cortex-M3 on QEMU.

## About

This project builds an RTOS from the ground up — starting with bare-metal boot code and UART output, and incrementally adding a task scheduler, synchronization primitives, memory management, and hardware drivers. The goal is to deeply understand how embedded systems work at the lowest level.

**Target Platform**: ARM Cortex-M3 (QEMU `lm3s6965evb`)

## Building & Running

### Prerequisites

- `arm-none-eabi-gcc` (ARM cross-compiler)
- `qemu-system-arm` (ARM system emulator)
- `gdb-multiarch` (debugger, optional)
- `make`

On Ubuntu/Debian:
```bash
sudo apt install gcc-arm-none-eabi qemu-system-arm gdb-multiarch make
```

### Build

```bash
make
```

### Run on QEMU

```bash
make run
```
Press `Ctrl-A` then `X` to exit QEMU.

### Debug with GDB

Terminal 1:
```bash
make debug
```

Terminal 2:
```bash
gdb-multiarch build/mini-rtos.elf -ex 'target remote :3333'
```

## Project Structure

```
cortex-m-rtos/
├── src/
│   ├── startup.s          # ARM Cortex-M3 vector table & boot code
│   ├── main.c             # Application entry point
│   └── hal/
│       └── uart.c         # UART driver (serial output)
├── include/
│   └── hal/
│       └── uart.h
├── Makefile
├── linker.ld              # Memory layout for LM3S6965
└── README.md
```

## Current Status

🟢 **Phase 1: Bare-Metal Boot** — UART output on QEMU

## License

MIT
