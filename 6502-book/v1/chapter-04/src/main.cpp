#include <iostream>
#include <iomanip>
#include "Memory.h"
#include "SystemBus.h"

int main() {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 4: The System Bus" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 1. Instantiate our underlying Memory subsystem and Interconnect Bus
    Memory memory;
    SystemBus bus(&memory);

    std::cout << "[Test 1] Standard RAM Routing Verification..." << std::endl;
    // Write value directly through the Bus into RAM address 0x0200
    bus.write(0x0200, 0x55);
    uint8_t ramRead = bus.read(0x0200);
    std::cout << "  Write 0x55 to $0200 -> Read back: 0x" 
              << std::hex << std::uppercase << (int)ramRead << std::endl;

    std::cout << "\n[Test 2] Keyboard Intercept & Strobe Latching Verification..." << std::endl;
    // Simulate the OS typing the character 'J' into the emulated computer
    bus.setKey('J');

    // Read the keyboard latch address $C000 to see if it correctly sets Bit 7 high
    uint8_t keyStrobeRead = bus.read(0xC000);
    std::cout << "  Read Keyboard at $C000 (Strobe Active): 0x" 
              << std::hex << (int)keyStrobeRead << std::endl;

    // Clear the strobe by reading $C010
    std::cout << "  Reading $C010 to acknowledge and clear the strobe..." << std::endl;
    uint8_t clearRead = bus.read(0xC010);
    std::cout << "  Read $C010 back: 0x" << std::hex << (int)clearRead << std::endl;

    // Read $C000 again to ensure the strobe Bit 7 was successfully cleared to 0
    uint8_t keyStrobeAfter = bus.read(0xC000);
    std::cout << "  Read Keyboard at $C000 (Strobe Cleared): 0x" 
              << std::hex << (int)keyStrobeAfter << std::endl;

    std::cout << "\n[Test 3] Speaker Tick (BEEP) Intercept Verification..." << std::endl;
    // Trigger the speaker via a read
    bus.read(0xC030);
    // Trigger the speaker via a write
    bus.write(0xC030, 0xFF);

    if (ramRead == 0x55 && (keyStrobeRead & 0x80) == 0x80 && (keyStrobeAfter & 0x80) == 0x00) {
        std::cout << "\n===========================================" << std::endl;
        std::cout << " System Interconnect Bus Verified SUCCESSFULLY." << std::endl;
        std::cout << " Ready for Chapter 5: The CPU." << std::endl;
        std::cout << "===========================================" << std::endl;
    } else {
        std::cout << "\n[ERROR] Bus Routing Verification FAILED!" << std::endl;
        return 1;
    }

    return 0;
}
