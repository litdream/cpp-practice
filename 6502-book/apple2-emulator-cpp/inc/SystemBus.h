#pragma once
#include "Bus.h"
#include "Memory.h"

class SystemBus : public Bus {
public:
    SystemBus(Memory* memory);
    ~SystemBus() override = default;

    uint8_t read(uint16_t addr) override;
    void write(uint16_t addr, uint8_t value) override;

    void setKey(uint8_t key);

private:
    Memory* memory;
    uint8_t currentKey = 0;
    bool keyStrobe = false;
};
