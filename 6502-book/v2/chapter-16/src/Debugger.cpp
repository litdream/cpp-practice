#include "Debugger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Debugger {

    uint16_t disassemble(Bus* bus, uint16_t addr, std::string& outAssembly) {
        uint8_t opcode = bus->read(addr);
        std::stringstream ss;
        ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << addr << ": ";

        // Highly simplified pedagogical disassembler supporting Chapter 5/8 Opcodes
        switch (opcode) {
            case 0xA9: {
                uint8_t val = bus->read(addr + 1);
                ss << "A9 " << std::setw(2) << (int)val << "     LDA #$" << std::setw(2) << (int)val;
                outAssembly = ss.str();
                return 2;
            }
            case 0x8D: {
                uint8_t lo = bus->read(addr + 1);
                uint8_t hi = bus->read(addr + 2);
                uint16_t target = (hi << 8) | lo;
                ss << "8D " << std::setw(2) << (int)lo << " " << std::setw(2) << (int)hi << "  STA $" << std::setw(4) << target;
                outAssembly = ss.str();
                return 3;
            }
            case 0x4C: {
                uint8_t lo = bus->read(addr + 1);
                uint8_t hi = bus->read(addr + 2);
                uint16_t target = (hi << 8) | lo;
                ss << "4C " << std::setw(2) << (int)lo << " " << std::setw(2) << (int)hi << "  JMP $" << std::setw(4) << target;
                outAssembly = ss.str();
                return 3;
            }
            case 0x62: // Fake placeholder opcode we saw in $FFFC for Reset Vector?
                       // Wait, Reset vector points to address, it's not an opcode.
                       // But if CPU jumps to $FA62, let's see what is at $FA62 in ROM.
                       // In Apple II+, $FA62 is the start of boot.
                ss << std::setw(2) << (int)opcode << "        ???";
                outAssembly = ss.str();
                return 1;
            default: {
                ss << std::hex << std::setw(2) << (int)opcode << "        .BYTE $" << std::setw(2) << (int)opcode;
                outAssembly = ss.str();
                return 1;
            }
        }
    }

    void dumpState(const CPU6502& cpu) {
        std::cout << "--- CPU State Inspection ---" << std::endl;
        std::cout << "  A:  0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)cpu.A << std::endl;
        std::cout << "  X:  0x" << std::hex << std::setw(2) << (int)cpu.X << std::endl;
        std::cout << "  Y:  0x" << std::hex << std::setw(2) << (int)cpu.Y << std::endl;
        std::cout << "  SP: 0x" << std::hex << std::setw(4) << (int)cpu.SP << std::endl;
        std::cout << "  PC: 0x" << std::hex << std::setw(4) << (int)cpu.PC << std::endl;
        
        // S is Status. ALU Flags are internal but we can read S.
        // Wait, AF_TO_EF loads flags from S, EF_TO_AF saves them.
        // During execution, internal variables (flagc, etc.) hold the flags.
        // Since we are outside CPU6502, we can only read cpu.S, but wait.
        // In CPU6502.cpp:
        // EF_TO_AF is called at the end of `execute`.
        // So `cpu.S` holds the correct status flags when the CPU is stopped!
        std::cout << "  S:  0x" << std::hex << std::setw(2) << (int)cpu.S << " (Flags: ";
        std::cout << ((cpu.S & 0x80) ? "N" : ".");
        std::cout << ((cpu.S & 0x40) ? "V" : ".");
        std::cout << "."; // Reserved
        std::cout << ((cpu.S & 0x10) ? "B" : ".");
        std::cout << ((cpu.S & 0x08) ? "D" : ".");
        std::cout << ((cpu.S & 0x04) ? "I" : ".");
        std::cout << ((cpu.S & 0x02) ? "Z" : ".");
        std::cout << ((cpu.S & 0x01) ? "C" : ".");
        std::cout << ")" << std::dec << std::endl;
        std::cout << "----------------------------" << std::endl;
    }

    void dumpMemory(Bus* bus, uint16_t startAddr, uint16_t size) {
        std::cout << "--- Memory Hex Dump ---" << std::endl;
        uint16_t endAddr = startAddr + size;
        // Align to 16 bytes
        startAddr &= 0xFFF0;
        
        for (uint32_t addr = startAddr; addr < endAddr; addr += 16) {
            std::cout << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << addr << ": ";
            
            // Hex bytes
            for (int col = 0; col < 16; ++col) {
                if (addr + col < endAddr) {
                    std::cout << std::setw(2) << (int)bus->read(addr + col) << " ";
                } else {
                    std::cout << "   ";
                }
            }
            
            // ASCII characters
            std::cout << " ";
            for (int col = 0; col < 16; ++col) {
                if (addr + col < endAddr) {
                    uint8_t ch = bus->read(addr + col) & 0x7F; // Remove high bit
                    if (ch >= 32 && ch <= 126) {
                        std::cout << (char)ch;
                    } else {
                        std::cout << ".";
                    }
                } else {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::dec << "-----------------------" << std::endl;
    }
}
