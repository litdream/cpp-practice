#include "Memory.h"
#include <cstring>
#include <iostream>
#include <iomanip>

Memory::Memory() {
    // Initialize our hardware memory ranges
    mainRAM.resize(48 * 1024, 0); // 48KB RAM
    rom.resize(12 * 1024, 0);     // 12KB ROM
    ioSpace.resize(256, 0);       // 256 bytes for $C000-$C0FF I/O
    slotROM.resize(15 * 256, 0);  // 3.75KB for $C100-$CFFF Slot ROMs

    // Pedagogical note: The Apple II monitor uses address $3F4 (Power-Up Signature)
    // during boots to determine whether to perform a cold boot or a warm start. 
    // Setting it to 0xFF breaks the signature, safely forcing a fresh Cold Boot.
    mainRAM[0x3F4] = 0xFF; 

    // Build our fast page-lookup tables
    updatePaging();
}

Memory::~Memory() {}

void Memory::updatePaging() {
    // 1. RAM: $0000 - $BFFF (Pages 0x00 through 0xBF)
    for (int page = 0x00; page <= 0xBF; ++page) {
        readPages[page] = mainRAM.data() + (page * 256);
        writePages[page] = readPages[page]; // RAM is Read & Write
    }

    // 2. I/O: $C000 - $C0FF (Page 0xC0)
    readPages[0xC0] = ioSpace.data();
    writePages[0xC0] = ioSpace.data(); 

    // 3. Peripheral Slot ROMs: $C100 - $CFFF (Pages 0xC1 - 0xCF)
    for (int page = 0xC1; page <= 0xCF; ++page) {
        readPages[page] = slotROM.data() + ((page - 0xC1) * 256);
        writePages[page] = nullptr; // ROM is read-only for standard CPU writes
    }

    // 4. ROM: $D000 - $FFFF (Pages 0xD0 through 0xFF)
    for (int page = 0xD0; page <= 0xFF; ++page) {
        readPages[page] = rom.data() + ((page - 0xD0) * 256);
        writePages[page] = nullptr; // ROM is strictly Read-Only
    }
}

uint8_t Memory::read(uint16_t addr) {
    uint8_t page = addr >> 8; // Extract the high-byte (Page Number)
    if (readPages[page]) {
        return readPages[page][addr & 0xFF]; // Offset by low-byte
    }
    return 0x00; // Return open-bus floating state (usually 0) if unmapped
}

void Memory::write(uint16_t addr, uint8_t value) {
    uint8_t page = addr >> 8; // Extract the high-byte (Page Number)
    if (writePages[page]) {
        writePages[page][addr & 0xFF] = value;
    }
    // Writes to Read-Only pages (where writePages[page] == nullptr) are silently discarded
}

void Memory::loadROM(const uint8_t* romData, size_t size) {
    if (size > rom.size()) {
        std::cerr << "ROM data too large, truncating to 12KB." << std::endl;
        size = rom.size();
    }
    std::memcpy(rom.data(), romData, size);
    updatePaging();
}

void Memory::writeSlotROM(uint16_t addr, uint8_t value) {
    if (addr >= 0xC100 && addr <= 0xCFFF) {
        size_t offset = addr - 0xC100;
        if (offset < slotROM.size()) {
            slotROM[offset] = value;
        }
    }
}
