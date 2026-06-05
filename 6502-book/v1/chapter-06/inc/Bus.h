#pragma once
#include <cstdint>

// Pure abstract interface representing our parallel copper trace bus lines
class Bus {
public:
    virtual ~Bus() = default;
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t addr, uint8_t value) = 0;
};
