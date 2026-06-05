#include "CPU6502.h"
#include "CPU6502_opcodes.h"
#include <iostream>

CPU6502::CPU6502(Bus* bus) : bus(bus), A(0), X(0), Y(0), S(0x24), PC(0), SP(0x01FF), flagc(0), flagn(0), flagv(0), flagz(0) {
    // Initialize flags or other state
}

CPU6502::~CPU6502() {
#if TRACE
    if (traceFile.is_open()) {
        traceFile.close();
    }
#endif
}

#if TRACE
void CPU6502::startTrace(uint32_t count) {
    if (!traceFile.is_open()) {
        traceFile.open("cpu_trace.log");
    }
    tracing = true;
    traceCount = count;
    std::cout << "Trace started for " << count << " instructions." << std::endl;
}
#endif

void CPU6502::reset() {
    // Read Reset Vector at $FFFC-$FFFD
    uint8_t low = bus->read(0xFFFC);
    uint8_t high = bus->read(0xFFFD);
    PC = (high << 8) | low;
    SP = 0x01FF; // Stack starts at $01FF
    S = 0x24; // Default status (Interrupt disabled, Reserved set)
    
    std::cout << "CPU Reset. PC set to: 0x" << std::hex << PC << std::dec << std::endl;
}

uint8_t CPU6502::read(uint16_t addr) {
    return bus->read(addr);
}

void CPU6502::write(uint16_t addr, uint8_t value) {
    bus->write(addr, value);
}

uint16_t CPU6502::read16(uint16_t addr) {
    return bus->read(addr) | (bus->read(addr + 1) << 8);
}

void CPU6502::push(uint8_t value) {
    bus->write(SP--, value);
    if (SP < 0x0100) SP = 0x01FF; // Wrap stack
}

uint8_t CPU6502::pop() {
    SP++;
    if (SP > 0x01FF) SP = 0x0100; // Wrap stack page 1
    return bus->read(SP);
}

uint32_t CPU6502::execute(uint32_t cycles) {
    uint16_t addr;
    uint8_t flagc = this->flagc;
    uint8_t flagn = this->flagn;
    uint8_t flagv = this->flagv;
    uint8_t flagz = this->flagz;
    uint16_t temp;
    uint16_t temp2;
    uint16_t val;
    bool jammed = false;
    
    AF_TO_EF; // Load flags from S
    
    uint32_t uExecutedCycles = 0;
    bool bSlowerOnPagecross = false;
    uint16_t base;
    int g_nIrqCheckTimeout = 16;

    while (uExecutedCycles < cycles && !jammed) {
        uint16_t uExtraCycles = 0;
        uint8_t iOpcode = bus->read(PC++);
        
#if TRACE
        if (tracing) {
            traceFile << "PC: " << std::hex << (PC-1) 
                      << " Op: " << (int)iOpcode 
                      << " A: " << (int)A 
                      << " X: " << (int)X 
                      << " Y: " << (int)Y 
                      << " S: " << (int)S 
                      << " SP: " << std::hex << SP << std::dec << std::endl;
            traceCount--;
            if (traceCount == 0) {
                tracing = false;
                traceFile.flush();
                std::cout << "Trace completed." << std::endl;
            }
        }
#endif

        #include "CPU6502_switch.h"
    }
    
    EF_TO_AF; // Save flags back to S
    
    this->flagc = flagc;
    this->flagn = flagn;
    this->flagv = flagv;
    this->flagz = flagz;

    return uExecutedCycles;
}
