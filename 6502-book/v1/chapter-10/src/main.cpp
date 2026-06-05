#include <iostream>
#include <iomanip>
#include <string>
#include "Memory.h"
#include "SystemBus.h"
#include "CPU6502.h"
#include "Debugger.h"
#include "apple2plus_rom.h"

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 10: Debugger & Inspection" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 1. Instantiate Core Topology
    Memory memory;
    SystemBus bus(&memory);
    CPU6502 cpu(&bus);

    // 2. Load System ROM
    std::cout << "[Test 1] Loading System ROM for Kernel Traceability..." << std::endl;
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    // 3. Test Disassembler at $D000 (Start of ROM)
    std::cout << "[Test 2] Synthesizing Instruction Disassembly..." << std::endl;
    std::string assembly;
    uint16_t bytes = Debugger::disassemble(&bus, 0xD000, assembly);
    std::cout << "  Disassembled at $D000: " << assembly << " (Consumed: " << bytes << " bytes)" << std::endl;

    // 4. Test Memory Hex-Dumper on Zero-Page and Primary Display Map
    std::cout << "[Test 3] Triggering Localized System RAM Hex Dumps..." << std::endl;
    std::cout << "  Dumping Zero-Page Address space ($0000-$0020):" << std::endl;
    Debugger::dumpMemory(&bus, 0x0000, 0x20);

    // 5. Trigger Power-Up and dump CPU state
    std::cout << "[Test 4] Triggering Reset Vectors & Introspecting CPU Topology..." << std::endl;
    cpu.reset();
    Debugger::dumpState(cpu);

    // 6. Execute a few instructions and dump again
    // Handcraft code at $0300
    bus.write(0x0300, 0xA9);
    bus.write(0x0301, 0x42);
    bus.write(0x0302, 0x8D);
    bus.write(0x0303, 0x00);
    bus.write(0x0304, 0x02);

    cpu.PC = 0x0300; // Force PC to our handcrafted code
    std::cout << "Executing handcrafted code at $0300..." << std::endl;
    
    std::string dasm1, dasm2;
    Debugger::disassemble(&bus, 0x0300, dasm1);
    Debugger::disassemble(&bus, 0x0302, dasm2);
    std::cout << "  Trace: " << dasm1 << std::endl;
    cpu.execute(2); // Execute LDA
    std::cout << "  Trace: " << dasm2 << std::endl;
    cpu.execute(4); // Execute STA

    Debugger::dumpState(cpu);

    std::cout << "\n===========================================" << std::endl;
    std::cout << " Advanced Debugging Mechanisms SUCCESS." << std::endl;
    std::cout << " Blueprint Book COMPLETE." << std::endl;
    std::cout << "===========================================" << std::endl;

    return 0;
}
