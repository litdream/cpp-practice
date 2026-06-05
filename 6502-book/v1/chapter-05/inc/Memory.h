#pragma once
#include <cstdint>
#include <vector>

class Memory {
public:
    Memory();
    ~Memory();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t value);

    // Load ROM image into $D000-$FFFF
    void loadROM(const uint8_t* romData, size_t size);

private:
    // Memory arrays modeled using standard vectors for safe bounds checking
    std::vector<uint8_t> mainRAM; // 48K ($0000 - $BFFF)
    std::vector<uint8_t> rom;     // 12K ($D000 - $FFFF)
    std::vector<uint8_t> ioSpace; // 256 bytes ($C000 - $C0FF)
    
    // Software Page Lookups (256 pages)
    uint8_t* readPages[256];
    uint8_t* writePages[256];

    void updatePaging();
};
