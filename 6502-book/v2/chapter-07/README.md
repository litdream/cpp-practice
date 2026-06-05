# Chapter 7: ROM Loading & Environment Bootstrap

## 📖 Overview & Educational Philosophy
Welcome to Chapter 7! You have synthesized the Central Processing Unit and configured the Discrete Video logic to render shape matrices to pixels. But a computer without its operating system is just inert copper and silicon. Today, we **flash the motherboard**.

In this chapter, we load the official, pre-compiled **12KB Apple II+ System ROM** (containing the Applesoft BASIC interpreter and Steve Wozniak's iconic System Monitor) into virtual Memory ranges $D000 through $FFFF.

## 🛠️ The Hardware Dance: EEPROM Mapping
On a physical 1977 motherboard, ROM is stored on discrete DIP chips mapped strictly to the highest addresses of the CPU Bus. By injecting the `Apple2plus_rom.h` matrix directly into our bounds-checked `Memory` vector, we simulate applying voltage to the physical address lines to query the system's foundational code.

## ⚙️ Building and Running Chapter 7
Our `main.cpp` initializes the system memory, loads the production 12KB ROM image, and queries historical ROM addresses to verify strict byte-for-byte fidelity!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-07
```
