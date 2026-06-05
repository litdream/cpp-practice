#pragma once
// #define TRACE 1
#define TRACE 0

#include <cstdint>
#include <fstream>
#include "Bus.h"

class CPU6502 {
public:
    CPU6502(Bus* bus);
    ~CPU6502();

    void reset();
    uint32_t execute(uint32_t cycles);

    // Registers
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t S; // Status
    uint16_t PC;
    uint16_t SP;

#if TRACE
    void startTrace(uint32_t count);
#endif

private:
    Bus* bus;
#if TRACE
    std::ofstream traceFile;
    bool tracing = false;
    uint32_t traceCount = 0;
#endif

    // Flags (separated for performance)
    uint8_t flagc;
    uint8_t flagn;
    uint8_t flagv;
    uint8_t flagz;

    // Helper methods for addressing modes, etc.
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);
    uint16_t read16(uint16_t addr);
    void push(uint8_t value);
    uint8_t pop();
};
