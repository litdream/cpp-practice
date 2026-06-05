#include <iostream>
#include <iomanip>
#include "Memory.h"
#include "SystemBus.h"
#include "apple2plus_rom.h"

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 7: ROM Loading" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 1. Instantiate our core memory interconnects
    Memory memory;
    SystemBus bus(&memory);

    std::cout << "[Test 1] Querying unmapped ROM state (Expect 0x00)..." << std::endl;
    uint8_t preRead = bus.read(0xD000);
    std::cout << "  Read at $D000 before load: 0x" 
              << std::hex << std::uppercase << (int)preRead << std::dec << std::endl;

    // 2. Load the actual production Apple II+ ROM
    std::cout << "[Test 2] Flashing 12KB System ROM into $D000-$FFFF..." << std::endl;
    // Note: sizeof(Apple2plus_rom) includes the null-terminator byte, so we subtract 1
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    // 3. Verify specific byte topographies within the ROM space
    std::cout << "[Test 3] Verifying ROM Byte-for-Byte Address Topologies..." << std::endl;

    // Start of ROM ($D000)
    uint8_t d000Read = bus.read(0xD000);
    uint8_t d000Expected = Apple2plus_rom[0];
    std::cout << "  Read at $D000: 0x" << std::hex << (int)d000Read 
              << " (Expected: 0x" << (int)d000Expected << ")" << std::endl;

    // A byte in the middle (e.g., $E000 -> offset 0x1000)
    uint8_t e000Read = bus.read(0xE000);
    uint8_t e000Expected = Apple2plus_rom[0x1000];
    std::cout << "  Read at $E000: 0x" << std::hex << (int)e000Read 
              << " (Expected: 0x" << (int)e000Expected << ")" << std::endl;

    // Verify Read-Only Protection (Writes to ROM should be discarded)
    std::cout << "[Test 4] Verifying ROM Write Protection (EEPROM Locking)..." << std::endl;
    bus.write(0xD000, 0xFF); // Try to overwrite $D000
    uint8_t afterWriteRead = bus.read(0xD000);
    std::cout << "  Write 0xFF to $D000 -> Read back: 0x" 
              << std::hex << (int)afterWriteRead << std::endl;

    if (d000Read == d000Expected && e000Read == e000Expected && afterWriteRead == d000Expected) {
        std::cout << "\n===========================================" << std::endl;
        std::cout << " System ROM Loaded & Protected SUCCESSFULLY." << std::endl;
        std::cout << " Ready for Chapter 8: The Reset Vector." << std::endl;
        std::cout << "===========================================" << std::endl;
    } else {
        std::cout << "\n[ERROR] ROM Loading or Protection Verification FAILED!" << std::endl;
        return 1;
    }

    return 0;
}
