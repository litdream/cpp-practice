# Chapter 3: Memory Subsystem Architecture

## 📖 Overview & Educational Philosophy
In physical hardware, an address bus is an array of copper traces carrying voltage highs (`1`) and lows (`0`). When the CPU needs to read or write a byte of data, it asserts an address on the bus, and the relevant physical memory chips (RAM or ROM) respond. 

In Chapter 3, we translate this complex electronic backplane into a pure, safe, and modern object-oriented software abstraction. We move away from dangerous raw memory pointers toward robust, bound-checked representations using C++ `std::vector` to model our **64KB (65,536 bytes)** addressable space.

## 🧠 Key Hardware Concepts: Paging the Map
The 6502 processor has a 16-bit address bus. This allows it to reference memory locations from `$0000` to `$FFFF`. To manage this space effectively in software, we partition the memory into **256 pages**, where each page is **256 bytes** long (`256 × 256 = 65,536`). 

We exploit this paging mechanism to create ultra-fast Software Page Tables (`readPages` and `writePages` lookups). This gives us precise control over hardware edge-cases, like making System ROM read-only in the upper addressing ranges (`$D000–$FFFF`).

### Critical Reserved Pages on the Apple II:
* **Zero-Page (`$00` / addresses `$0000–$00FF`)**: High-speed internal register array for the 6502.
* **Stack Page (`$01` / addresses `$0100–$01FF`)**: Hardcoded execution boundary of the 6502 hardware stack.
* **Display Text Page 1 (`$0400–$07FF`)**: Memory mapped to the hardware text-generation circuit.
* **Peripheral I/O and Soft Switches (`$C000–$C0FF`)**: Addresses used to poll keyboard keystrobes or trigger audio ticks.

## ⚙️ Building and Running Chapter 3
Our `main.cpp` executes a comprehensive automated verification suite against our new `Memory` subsystem, proving that:
1. Standard RAM reads and writes function perfectly.
2. Upper System ROM (`$D000–$FFFF`) correctly rejects write attempts.
3. The power-up signature at address `$03F4` is safely initialized to force a cold boot.

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-03
```
