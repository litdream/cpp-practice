#pragma once
#include "Bus.h"
class Memory;
class Speaker;

class SystemBus : public Bus {
public:
    SystemBus(Memory* memory, Speaker* speaker = nullptr);
    ~SystemBus() override = default;

    // Bus Routing
    uint8_t read(uint16_t addr) override;
    void write(uint16_t addr, uint8_t value) override;
    void tick(uint32_t cycles) override;

    // Peripheral Interconnects
    void setKey(uint8_t key);

    uint64_t getSystemCycles() const { return systemCycles; }

private:
    Memory* memory;
    Speaker* speaker;
    uint8_t currentKey = 0;
    bool keyStrobe = false;
    uint64_t systemCycles = 0;
};
