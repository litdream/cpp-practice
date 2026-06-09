#include "CPU6502.h"
#include "CPU6502_opcodes.h"
#include "SystemBus.h"
#include <iostream>
#include <iomanip>

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
    
    SystemBus* sysBus = dynamic_cast<SystemBus*>(bus);
    DiskDrive* disk = sysBus ? sysBus->getDiskDrive() : nullptr;

    uint32_t uExecutedCycles = 0;
    bool bSlowerOnPagecross = false;
    uint16_t base;
    int g_nIrqCheckTimeout = 16;

    while (uExecutedCycles < cycles && !jammed) {

        // HLE Boot ROM Intercepts
        if (PC == 0xC600) {
            std::cout << "[HLE Boot] Intercepted Slot 6 Boot at $C600" << std::endl;
            if (disk) {
                // 1. Load Sector 0 to $0800-$08FF
                std::vector<uint8_t> sector0 = disk->readSector(0, 0);
                for (int i = 0; i < 256; ++i) {
                    bus->write(0x0800 + i, sector0[i]);
                }


                // 2. Populate GCR-to-6bit translation table in RAM at $0356-$03FF (base $02D6)
                // This mimics what the physical Slot 6 ROM bootloader routine does before jumping to $0801,
                // enabling the custom 6502 game loader's LLE GCR reader to successfully decode disk data!
                const uint8_t GCR_WRITE_TABLE[64] = {
                    0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6, 
                    0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3, 
                    0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc, 
                    0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3, 
                    0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 
                    0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec, 
                    0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
                    0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff 
                };
                for (int i = 0; i < 64; ++i) {
                    bus->write(0x02D6 + GCR_WRITE_TABLE[i], i);
                }
                std::cout << "[HLE Boot] Populated GCR-to-6bit translation table in RAM." << std::endl;
            } else {
                std::cerr << "[HLE Boot] Error: No virtual disk drive available!" << std::endl;
            }
            X = 0x60;
            Y = 0x00;
            bus->write(0x2B, 0x60);
            PC = 0x0801; 
            continue; 
        }



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
