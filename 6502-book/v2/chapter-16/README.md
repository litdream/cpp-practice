# Chapter 16: Floppy Disk Subsystem & HLE Booting

## 📖 Overview & Educational Philosophy
Welcome to Chapter 16! In this chapter, we take on a highly challenging and rewarding problem in system design: emulating a **floppy disk drive and controller** and successfully booting a real historical disk image—**`SNAKEBYTE.DSK`**!

Emulating physical disk drives is notoriously complex due to mechanical state machines, timing loops, and low-level GCR (Group Code Recording) encoding. To make this educational and approachable, we implement a **High-Level Emulation (HLE)** approach for the Disk II Slot 6 controller ROM, paired with a **Low-Level Emulation (LLE)** approach for stepper motor tracking. 

This hybrid method allows us to:
1. Understand mechanical disk head positioning and stepper motor phases.
2. Resolve disk sector layout and physical-to-logical sector interleaving.
3. Successfully load and execute a raw disk image by intercepting the bios sector read routine `$C65C` in C++.

## 🛠️ The Hardware Dance: Stepper Motors & Sector Interleaving

### 1. Stepper Motor Tracking
The Apple Disk II is a "dumb" drive. To move the read head, the CPU must manually toggle 4 electromagnetic stepper phases (`$C0E0-$C0E7` for Slot 6). By energizing these phases in sequence (0 -> 1 -> 2 -> 3), the head moves inward or outward by **half-track steps**. 
In this chapter, we implement a C++ head position tracker inside `DiskDrive.cpp` that monitors these phase transitions to maintain the head's exact current track index (0 to 34).

### 2. Sector Interleaving (Skewing)
Floppy disk sectors are not physically contiguous (e.g. Sector 1 is not next to Sector 0 on the disk). This is because the CPU needs time to process data before the next sector spins under the head. 
- A `.dsk` file is stored in **Logical sector order** (for software ease).
- The disk controller reads **Physical sectors**.
When our HLE routine is asked to read a physical sector `P`, we must convert it to the corresponding logical sector `L` to fetch the correct 256 bytes from our `.dsk` file:
```cpp
const int DOS_PHYSICAL_TO_LOGICAL[16] = {
    0, 13, 11, 9, 7, 5, 3, 1, 14, 12, 10, 8, 6, 4, 2, 15
};
```

### 3. HLE Boot ROM Intercepts
Instead of emulating raw bit shift registers and GCR data streams, we intercept the CPU at key entry points of the Slot 6 ROM:
*   **`$C600` (Boot Sector 0 Load):** Intercepted to automatically load Track 0, Sector 0 into memory at `$0800` and jump the CPU's `PC` straight to `$0801`.
*   **`$C65C` (Sector Read):** Intercepted to read a 256-byte sector `Y` from the current track (tracked by the stepper motor) into the memory page indicated by `$27`, then simulate a `RTS` instruction to return control back to the calling loader.

## ⚙️ Building and Running Chapter 16

Make sure that `SNAKEBYTE.DSK` is present in your build execution directory!

### Build Steps:
```bash
mkdir -p build
cmake -S . -B build
cmake --build build
./build/chapter-16
```

### Emulator Controls:
*   **`[F1]`** - Print the current CPU register states to the terminal.
*   **`[F2]`** - Dump the Zero-Page memory range to the terminal.
*   **`[F3]`** - Force a manual boot from Slot 6 (resets the CPU and jumps to `$C600`).
*   **`[ESC]`** - Exit the emulator.
