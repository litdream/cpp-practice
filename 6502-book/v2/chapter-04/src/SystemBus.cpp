#include "SystemBus.h"
#include <iostream>
#include <iomanip>

SystemBus::SystemBus(Memory* memory) : memory(memory) {}

uint8_t SystemBus::read(uint16_t addr) {
    // Intercept Memory-Mapped I/O (MMIO) Registers
    
    // 1. $C000: Apple II Keyboard Strobe Latch
    if (addr == 0xC000) {
        if (keyStrobe) {
            std::cout << "  [Bus Event] CPU Polled Key: '" 
                      << (char)(currentKey & 0x7F) << "' (0x" 
                      << std::hex << (int)currentKey << std::dec << ")" 
                      << std::endl;
        }
        // Return the current key, with bit 7 set high if a key is available (Strobe)
        return currentKey | (keyStrobe ? 0x80 : 0x00);
    }

    // 2. $C010: Keyboard Strobe Clear
    if (addr == 0xC010) {
        keyStrobe = false; // Acknowledge and clear the strobe
        return currentKey;
    }

    // 3. $C030: Apple II Speaker Tick Toggle
    if (addr == 0xC030) {
        std::cout << "  [Bus Event] [BEEP]" << std::endl;
        return 0x00;
    }

    // Default: Route the read directly to System RAM/ROM
    return memory->read(addr);
}

void SystemBus::write(uint16_t addr, uint8_t value) {
    // Intercept MMIO Writes
    
    // 1. $C010: Keyboard Strobe Clear (Write also clears it)
    if (addr == 0xC010) {
        keyStrobe = false;
        return;
    }

    // 2. $C030: Speaker Tick Toggle
    if (addr == 0xC030) {
        std::cout << "  [Bus Event] [BEEP]" << std::endl;
        return;
    }

    // Default: Route the write directly to System RAM/ROM
    memory->write(addr, value);
}

void SystemBus::setKey(uint8_t key) {
    currentKey = key;
    keyStrobe = true; // Raise the strobe high
}
