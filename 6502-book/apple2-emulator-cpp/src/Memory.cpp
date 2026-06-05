#include "Memory.h"
#include <cstring>
#include <iostream>

Memory::Memory() {
    mainRAM.resize(48 * 1024, 0);
    rom.resize(12 * 1024, 0);
    ioSpace.resize(256, 0);
    
    // Force cold boot by breaking power-up signature at $3F4
    mainRAM[0x3F4] = 0xFF; 
    
    updatePaging();
}

Memory::~Memory() {}

void Memory::updatePaging() {
    // RAM: $0000 - $BFFF (Pages 0x00 - 0xBF)
    for (int page = 0x00; page <= 0xBF; ++page) {
        readPages[page] = mainRAM.data() + (page * 256);
        writePages[page] = readPages[page];
    }

    // I/O: $C000 - $C0FF (Page 0xC0)
    readPages[0xC0] = ioSpace.data();
    writePages[0xC0] = ioSpace.data(); // Temp, will need proper handlers

    // Empty/Peripheral ROMs: $C100 - $CFFF (Pages 0xC1 - 0xCF)
    for (int page = 0xC1; page <= 0xCF; ++page) {
        readPages[page] = nullptr;
        writePages[page] = nullptr;
    }

    // ROM: $D000 - $FFFF (Pages 0xD0 - 0xFF)
    for (int page = 0xD0; page <= 0xFF; ++page) {
        readPages[page] = rom.data() + ((page - 0xD0) * 256);
        writePages[page] = nullptr; // ROM is read-only
    }
}

uint8_t Memory::read(uint16_t addr) {
    uint8_t page = addr >> 8;
    if (readPages[page]) {
        return readPages[page][addr & 0xFF];
    }
    return 0;
}

void Memory::write(uint16_t addr, uint8_t value) {
    uint8_t page = addr >> 8;
    if (writePages[page]) {
        writePages[page][addr & 0xFF] = value;
    }
}

void Memory::loadROM(const uint8_t* romData, size_t size) {
    if (size > rom.size()) {
        std::cerr << "ROM data too large, truncating to 12KB" << std::endl;
        size = rom.size();
    }
    std::memcpy(rom.data(), romData, size);
    updatePaging();
}
