# Chapter 10: Advanced Debugging and System Inspection Mechanics

## 📖 Overview & Educational Philosophy
Welcome to Chapter 10! You have completed the entire hardware layout and environment synchronization topology of our 1977 Apple II+ emulator. But modern systems programming demands introspection. Today, we build the **x-ray specs**: the **Developer Debugging Panel**.

In this chapter, we construct localized software instrumentation hooks directly inside the Core Topology:
1. **Instruction Disassembler**: Converts Fetch execution streams back into readable assembly text (e.g., `A9 42 -> LDA #$42`).
2. **State Inspection Panel**: Dumps real-time Accumulator, Index Register, Stack Pointer, and ALU Status condition flags.
3. **Memory Hex-Dumper**: Prints localized pages of virtual RAM to track Zero-Page and Display address mutations.

## 🛠️ The Hardware Dance: Observability
Observability is a structural requirement. By intercepting execution states and printing addresses alongside instruction mnemonics, students learn to diagnose compound hardware bugs (invalid branches, stack wraps, or corrupt registers).

## ⚙️ Building and Running Chapter 10
Our `main.cpp` executes the system boot sequence and triggers all three developer introspection tools, then launches into the final, complete interactive OS GUI window for live execution and user input!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-10
```
