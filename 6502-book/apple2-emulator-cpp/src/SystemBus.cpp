#include <iostream>
#include "SystemBus.h"

SystemBus::SystemBus(Memory* memory) : memory(memory) {}

uint8_t SystemBus::read(uint16_t addr) {
    if (addr == 0xC000) {
        if (keyStrobe) {
            std::cout << "CPU Read Key: '" << (char)(currentKey & 0x7F) << "' (0x" << std::hex << (int)currentKey << std::dec << ")" << std::endl;
        }
        return currentKey | (keyStrobe ? 0x80 : 0x00);
    }
    if (addr == 0xC010) {
        keyStrobe = false;
        return currentKey; // Clear strobe on read
    }

    if (addr == 0xC030) {
        std::cout << "[BEEP]" << std::endl;
        return 0;
    }

    // Delegate other reads to memory
    return memory->read(addr);
}

void SystemBus::write(uint16_t addr, uint8_t value) {
    if (addr == 0xC010) {
        keyStrobe = false; // Clear strobe on write
        return;
    }

    if (addr == 0xC030) {
        std::cout << "[BEEP]" << std::endl;
        return;
    }

    // Delegate other writes to memory
    memory->write(addr, value);
}

void SystemBus::setKey(uint8_t key) {
    currentKey = key;
    keyStrobe = true;
}
