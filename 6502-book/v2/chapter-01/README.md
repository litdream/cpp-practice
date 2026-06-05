# Chapter 1: Introduction, Architecture, and Foundations

## 📖 Overview & Educational Philosophy
Welcome to the first chapter of this book! Rather than beginning our journey through the typically painstaking trial-and-error process of implementing a 6502 CPU from scratch, we are taking a pedagogical shortcut—what we call the **"Reverse-Engineering for Pedagogy"** strategy. 

In this chapter, we establish the foundational infrastructure of our Apple II emulator and ingest three critical, production-tested components borrowed directly from the open-source **AppleWin** project:
- **`apple2plus_rom.h`**: The system ROM mapping of the original Apple II+ hardware.
- **`CPU6502_opcodes.h`**: The complete opcode verification matrix.
- **`CPU6502_switch.h`**: The opcode execution dispatch switch statement.

By adopting these verified components, you can bypass months of laborious opcode debugging and focus directly on the systems architecture: the memory bus interconnect, physical memory mapping, and hardware subsystem synchronization.

## 🛠️ Hardware Specification Context
The historical Apple II computer, introduced in 1977, was powered by the MOS Technology 6502 processor running at approximately 1.023 MHz. It featured:
- **48KB to 64KB** of System RAM.
- **12KB** of System ROM holding Applesoft BASIC and the System Monitor.
- **Memory-Mapped I/O (MMIO)** in the `$C000–$CFFF` address space.

## ⚙️ Environment Setup & Verification
We are writing our emulator in modern **C++ (C++17/20 standard)** and leveraging **SDL2 (Simple DirectMedia Layer)** to handle our pixel buffers and host keyboard input integration in later chapters.

For this chapter, your primary objective is to verify that your C++ build environment is operational and configured with the required external libraries.

### Verification Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-01
```
