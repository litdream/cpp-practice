# Chapter 8: Power Up and the System Reset Vector

## 📖 Overview & Educational Philosophy
Welcome to Chapter 8! You have loaded the official System ROM operating TOPOLOGIES into our EEPROM mapping spaces. Now, it is time for the **spark of life**: the **Power-Up Electrical Sequence**.

In this chapter, we simulate the hardware microcode routine of the MOS 6502 processor upon receiving system power or a Reset strobe. The CPU queries addresses $FFFC and $FFFD, fetching the 16-bit Reset Vector pointing to the system operating monitor initialization routine.

## 🛠️ The Hardware Dance: Booting the Kernel
Upon electrical power-on, the CPU state is undefined. By triggering `cpu.reset()`, the CPU pulls the Reset Vector address ($FA62 in the Apple II+), setting the Program Counter to this location. As we clock the emulator, the CPU executes thousands of ROM firmware instructions: configuring the Zero-Page, testing system RAM, clearing the video page, and printing the iconic `Apple ][` banner!

## ⚙️ Building and Running Chapter 8
Our `main.cpp` executes the system boot sequence, clocks the virtual CPU for sufficient cycles to allow firmware initialization to stabilize, and verifies that the primary Text Page $0400 correctly holds the `Apple ][` monitor banner!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-08
```
