# Chapter 5: The Central Processing Unit (MOS 6502 Engine)

## 📖 Overview & Educational Philosophy
Welcome to Chapter 5! You have established the virtual monitor casing, configured the OS memory paging topology, and laid out the copper interconnect traces of our system bus. Now, it is time to build the **heart and mind** of the Apple II: the **MOS Technology 6502 CPU**.

In this chapter, we encapsulate the complete execution state, internal registry array, ALU Arithmetic flags, and instruction decoding toplines of the processor.

## 🛠️ The Hardware Dance: Fetch, Decode, Execute
Operating at exactly 1.023 MHz, the 6502 ticks through a strict topological cycle for every byte of code it processes:
1. **Fetch**: Query the interconnect `Bus` for the byte at the address held in the Program Counter (`PC`).
2. **Decode**: Map the opcode into our production-audited `CPU6502_switch.h` execution branching matrix.
3. **Execute**: Mutate internal data registers (`A`, `X`, `Y`), manipulate the hardware Stack Pointer (`SP`), calculate Arithmetic ALU Flag states (`P`), and advance the clock cycle counters.

## ⚙️ Building and Running Chapter 5
Our `main.cpp` executes a comprehensive emulation test suite. It initializes the virtual CPU, sets the Reset Vector in ROM to a specific RAM address containing a handcrafted 6502 machine code program (`LDA #$42; STA $0200`), and verifies that the CPU successfully fetches, executes, and mutates System RAM!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-05
```
