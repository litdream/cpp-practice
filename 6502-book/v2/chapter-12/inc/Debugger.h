#pragma once
#include <cstdint>
#include <string>
#include "Bus.h"
#include "CPU6502.h"

namespace Debugger {
    // 1. Disassemble a single instruction at a given address
    // Returns the number of bytes consumed by the instruction
    uint16_t disassemble(Bus* bus, uint16_t addr, std::string& outAssembly);

    // 2. Dump CPU registers and ALU status condition flags
    void dumpState(const CPU6502& cpu);

    // 3. Dump a localized page of System RAM to standard output
    void dumpMemory(Bus* bus, uint16_t startAddr, uint16_t size);
}
