#pragma once
#include "Bus.h"
#include "DiskDrive.h"
class Memory;
class Speaker;

class SystemBus : public Bus {
public:
    SystemBus(Memory* memory, Speaker* speaker = nullptr, DiskDrive* diskDrive = nullptr);
    ~SystemBus() override = default;

    // Bus Routing
    uint8_t read(uint16_t addr) override;
    void write(uint16_t addr, uint8_t value) override;
    void tick(uint32_t cycles) override;

    // Peripheral Interconnects
    void setKey(uint8_t key);

    uint64_t getSystemCycles() const { return systemCycles; }

    // Video Mode Getters
    bool isGraphicsMode() const { return isGraphics; }
    bool isMixedMode() const { return isMixed; }
    bool isPage2Mode() const { return isPage2; }
    bool isHiresMode() const { return isHires; }

    DiskDrive* getDiskDrive() const { return diskDrive; }

private:
    void handleVideoSwitch(uint16_t addr);

    Memory* memory;
    Speaker* speaker;
    DiskDrive* diskDrive;
    uint8_t currentKey = 0;
    bool keyStrobe = false;
    uint64_t systemCycles = 0;

    // Video Mode States
    bool isGraphics = false;
    bool isMixed = false;
    bool isPage2 = false;
    bool isHires = false;
};
