#include <iostream>
#include <iomanip>
#include "Memory.h"
#include "apple2plus_rom.h"

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 3: Memory Architecture" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 1. Initialize our emulated Memory Subsystem
    Memory memory;

    std::cout << "[Test 1] Standard RAM Read/Write Verification..." << std::endl;
    // Let's write a recognizable value (0x42) into Zero-Page RAM address 0x00FF
    memory.write(0x00FF, 0x42);
    uint8_t zpRead = memory.read(0x00FF);
    std::cout << "  Write 0x42 to $00FF -> Read back: 0x" 
              << std::hex << std::uppercase << (int)zpRead << std::endl;

    // Let's write into the Text Page 1 address 0x0400 (Top-left character of monitor)
    memory.write(0x0400, 'A'); 
    uint8_t textRead = memory.read(0x0400);
    std::cout << "  Write 'A' to $0400 -> Read back: '" 
              << (char)textRead << "'" << std::endl;

    std::cout << "\n[Test 2] Apple II+ Cold Boot Vector Verification..." << std::endl;
    // Read the power-up cold boot signature at 0x03F4
    uint8_t bootSig = memory.read(0x03F4);
    std::cout << "  Cold Boot Signature at $03F4: 0x" 
              << std::hex << (int)bootSig 
              << " (Expected: 0xFF to force cold boot)" << std::endl;

    std::cout << "\n[Test 3] System ROM Load & Read-Only Protection..." << std::endl;
    // Load the actual 12KB Apple II+ ROM mapped to $D000 - $FFFF
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    // Read a byte from the ROM space (e.g., the Reset Vector low-byte at 0xFFFC)
    uint8_t romRead = memory.read(0xFFFC);
    std::cout << "  Read Byte from $FFFC (Reset Vector Low): 0x" 
              << std::hex << (int)romRead << std::endl;

    // Attempt a illegal write into the read-only System ROM
    std::cout << "  Attempting to write 0xEA (NOP) to ROM address $FFFC..." << std::endl;
    memory.write(0xFFFC, 0xEA);
    
    // Read back again to see if the write was successfully discarded
    uint8_t romReadAfter = memory.read(0xFFFC);
    std::cout << "  Read Byte from $FFFC after write attempt: 0x" 
              << std::hex << (int)romReadAfter << std::endl;

    if (romRead == romReadAfter && zpRead == 0x42) {
        std::cout << "\n===========================================" << std::endl;
        std::cout << " Memory Subsystem Verified SUCCESSFULLY." << std::endl;
        std::cout << " Ready for Chapter 4: The System Bus." << std::endl;
        std::cout << "===========================================" << std::endl;
    } else {
        std::cout << "\n[ERROR] Memory Verification FAILED!" << std::endl;
        return 1;
    }

    return 0;
}
