#include <iostream>
#include <iomanip>
#include <vector>
#include "Memory.h"
#include "SystemBus.h"
#include "CPU6502.h"

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 5: The CPU" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 1. Instantiate our underlying Memory subsystem, Interconnect Bus, and CPU
    Memory memory;
    SystemBus bus(&memory);
    CPU6502 cpu(&bus);

    // 2. Prepare a 12KB ROM image containing a specific Reset Vector
    // In Apple II, ROM is $D000 - $FFFF (12KB).
    std::vector<uint8_t> fakeROM(12 * 1024, 0);

    // Set Reset Vector at $FFFC-$FFFD (which is $2FFC-$2FFD in the ROM array)
    // Point Reset Vector to $0300 in RAM
    uint16_t programStartAddress = 0x0300;
    fakeROM[0x2FFC] = programStartAddress & 0xFF;        // Low byte
    fakeROM[0x2FFD] = (programStartAddress >> 8) & 0xFF; // High byte

    std::cout << "[Test 1] Loading Custom ROM with Reset Vector -> $0300..." << std::endl;
    memory.loadROM(fakeROM.data(), fakeROM.size());

    // 3. Handcraft a 6502 machine code program and write it into System RAM at $0300
    // LDA #$42
    // STA $0200
    std::cout << "[Test 2] Writing 6502 Assembly Program (LDA #$42; STA $0200) into RAM..." << std::endl;
    bus.write(programStartAddress + 0, 0xA9); // LDA Immediate
    bus.write(programStartAddress + 1, 0x42); // Value 0x42
    bus.write(programStartAddress + 2, 0x8D); // STA Absolute
    bus.write(programStartAddress + 3, 0x00); // Low Byte of $0200
    bus.write(programStartAddress + 4, 0x02); // High Byte of $0200

    // 4. Reset the CPU to trigger the Fetch/Decode/Execute cycle!
    std::cout << "[Test 3] Triggering CPU Reset Vector Electrical Sequence..." << std::endl;
    cpu.reset();

    if (cpu.PC != programStartAddress) {
        std::cerr << "[ERROR] CPU PC not set to program start address: 0x" 
                  << std::hex << (int)cpu.PC << " (expected: 0x" << programStartAddress << ")" << std::endl;
        return 1;
    }

    // 5. Execute 6 cycles (LDA #$42 = 2, STA $0200 = 4)
    std::cout << "[Test 4] Clocking CPU to Fetch, Decode, and Execute Opcodes..." << std::endl;
    cpu.execute(6);

    // 6. Verify that our system memory was mutated correctly!
    uint8_t ramMutate = bus.read(0x0200);
    std::cout << "  Value at $0200 after execution: 0x" 
              << std::hex << std::uppercase << (int)ramMutate << std::dec << std::endl;

    if (ramMutate == 0x42 && cpu.A == 0x42) {
        std::cout << "\n===========================================" << std::endl;
        std::cout << " Central Processing Unit (MOS 6502) SUCCESS." << std::endl;
        std::cout << " Ready for Chapter 6: Video Architecture." << std::endl;
        std::cout << "===========================================" << std::endl;
    } else {
        std::cout << "\n[ERROR] CPU Execution or Memory Mutation FAILED!" << std::endl;
        std::cout << "  Expected: $0200 = 0x42, A = 0x42" << std::endl;
        std::cout << "  Actual:   $0200 = 0x" << std::hex << (int)ramMutate << ", A = 0x" << (int)cpu.A << std::dec << std::endl;
        return 1;
    }

    return 0;
}
