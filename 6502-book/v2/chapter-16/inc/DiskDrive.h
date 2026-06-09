#pragma once
#include <string>
#include <vector>
#include <cstdint>

class DiskDrive {
public:
    DiskDrive();
    ~DiskDrive() = default;

    bool loadDisk(const std::string& filepath);
    void writeSwitch(uint16_t addr, uint8_t value);
    uint8_t readSwitch(uint16_t addr, uint64_t systemCycles);

    int getCurrentTrack() const { return currentHalfTrack / 2; }
    bool isMotorOn() const { return motorOn; }

    std::vector<uint8_t> readSector(int track, int physicalSector);

private:
    void moveHead(int steps);
    void nibblizeDisk();
    void nibblizeTrack(int track, uint8_t* outBuffer);

    std::vector<uint8_t> diskData; // Raw .dsk file data (143,360 bytes)
    bool hasDisk = false;

    int currentHalfTrack = 0; // 0 to 79 (40 tracks * 2 half-tracks)
    bool motorOn = false;
    int magnetStates = 0; // Bitmask of active stepper phases (0-3)
    bool writeMode = false; // Q7 state
    bool shiftMode = false; // Q6 state
    int activeDrive = 1;

    // Physical Track representation in GCR format
    struct NibbledTrack {
        uint8_t bytes[6400];
    };
    std::vector<NibbledTrack> nibbledTracks;

    // Latch Synchronization
    uint64_t lastByteIndex = 0;
    bool byteReady = false;

    // DOS 3.3 Physical Sector to Logical Sector translation
    static const int DOS_PHYSICAL_TO_LOGICAL[16];
    // GCR 6-and-2 Write Translation Table
    static const uint8_t GCR_WRITE_TABLE[64];
};
