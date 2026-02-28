# FridgeOS

A minimal 32-bit operating system, made to learn about operating systems.

## Prerequisites
- NASM
- QEMU (for running the image)
- GNU i686-elf toolchain

## Build and Run
1. From PowerShell, execute:
   ```pwsh
   ./run.ps1
   ```
2. The script builds the boot sector and kernel into `build/os.img` and launches QEMU.

## Project Layout
- `boot/` - boot sector assembly.
- `kernel/` - freestanding kernel sources.
- `build/` - build artifacts (ignored in version control).