#include "DiskDrive.h"
#include <iostream>
#include <fstream>
#include <cstring>

const int DiskDrive::DOS_PHYSICAL_TO_LOGICAL[16] = {
    0, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 15
};

const uint8_t DiskDrive::GCR_WRITE_TABLE[64] = {
    0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6, 
    0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3, 
    0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc, 
    0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3, 
    0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 
    0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec, 
    0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
    0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff 
};

DiskDrive::DiskDrive() {
    magnetStates = 0;
}

bool DiskDrive::loadDisk(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "[DiskDrive] Failed to open disk image: " << filepath << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size != 143360) {
        std::cerr << "[DiskDrive] Invalid disk image size: " << size << " bytes (expected 143360)." << std::endl;
        return false;
    }

    diskData.resize(size);
    file.read((char*)diskData.data(), size);
    hasDisk = true;
    std::cout << "[DiskDrive] Successfully loaded disk image: " << filepath << " (140KB)" << std::endl;

    // Convert the logical sector image into physical GCR tracks
    nibblizeDisk();
    return true;
}

void DiskDrive::writeSwitch(uint16_t addr, uint8_t value) {
    int phase = (addr >> 1) & 3;
    bool state = (addr & 1) != 0;

    if (addr >= 0 && addr <= 7) {
        // Update the magnet states bitmask
        int phase_bit = (1 << phase);
        if (state) {
            magnetStates |= phase_bit;  // phase on
        } else {
            magnetStates &= ~phase_bit; // phase off
        }

        // Evaluate stepping effect based on active magnets relative to current head position
        int direction = 0;
        if (magnetStates & (1 << ((currentHalfTrack + 1) & 3))) {
            direction += 1;
        }
        if (magnetStates & (1 << ((currentHalfTrack + 3) & 3))) {
            direction -= 1;
        }

        if (direction) {
            moveHead(direction);
        }
    } else {
        switch (addr) {
            case 8: // $C0E8: Motor OFF
                if (motorOn) {
                    motorOn = false;
                    std::cout << "[DiskDrive] Motor OFF" << std::endl;
                }
                break;
            case 9: // $C0E9: Motor ON
                if (!motorOn) {
                    motorOn = true;
                    std::cout << "[DiskDrive] Motor ON" << std::endl;
                }
                break;
            case 10: // $C0EA: Drive 1 Select
                activeDrive = 1;
                std::cout << "[DiskDrive] Selected Drive 1" << std::endl;
                break;
            case 11: // $C0EB: Drive 2 Select
                activeDrive = 2;
                std::cout << "[DiskDrive] Selected Drive 2" << std::endl;
                break;
            case 12: // $C0EC: Q6L (Shift Mode)
                shiftMode = false;
                break;
            case 13: // $C0ED: Q6H (Latch Mode)
                shiftMode = true;
                break;
            case 14: // $C0EE: Q7L (Read Mode)
                writeMode = false;
                break;
            case 15: // $C0EF: Q7H (Write Mode)
                writeMode = true;
                break;
        }
    }
}

uint8_t DiskDrive::readSwitch(uint16_t addr, uint64_t systemCycles) {
    writeSwitch(addr, 0);

    if (addr == 12 || addr == 13) { // $C0EC or $C0ED
        if (writeMode) {
            // Write-protect check (Return 0x00 = writeable)
            return 0x00; 
        }
        
        if (addr == 12 && hasDisk && motorOn) { // $C0EC Read Shift Register
            // Apple II disk spins at 300 RPM (200ms per rotation).
            // A track has 6400 GCR bytes.
            // 200,000 microseconds / 6400 = 31.25 microseconds per GCR byte.
            // At 1.023 MHz, 1 cycle = 0.9775 microseconds.
            // 31.25 / 0.9775 = 31.97 (~32) cycles per byte.
            uint64_t currentByteIndex = (systemCycles / 32) % 6400;
            
            if (currentByteIndex != lastByteIndex) {
                lastByteIndex = currentByteIndex;
                byteReady = true;
            }

            int track = getCurrentTrack();
            if (track >= 0 && track < 35) {
                uint8_t trackByte = nibbledTracks[track].bytes[currentByteIndex];
                if (byteReady) {
                    byteReady = false; // Clear on read
                    return trackByte;  // Returns GCR byte (MSB is always 1)
                } else {
                    return trackByte & 0x7F; // Clear MSB to signal not ready
                }
            }
        }
    }
    return 0x00;
}

void DiskDrive::moveHead(int steps) {
    int oldTrack = getCurrentTrack();
    currentHalfTrack += steps;
    if (currentHalfTrack < 0) currentHalfTrack = 0;
    if (currentHalfTrack > 79) currentHalfTrack = 79;

    int newTrack = getCurrentTrack();
    if (newTrack != oldTrack) {
        std::cout << "[DiskDrive] Stepper Motor Active. Head moved to half-track " 
                  << currentHalfTrack << " (Track " << newTrack << ")" << std::endl;
    }
}

std::vector<uint8_t> DiskDrive::readSector(int track, int physicalSector) {
    std::vector<uint8_t> sector(256, 0);
    if (!hasDisk) return sector;

    size_t offset = (track * 16 + physicalSector) * 256;

    if (offset + 256 <= diskData.size()) {
        std::memcpy(sector.data(), &diskData[offset], 256);
    }
    return sector;
}

void DiskDrive::nibblizeDisk() {
    nibbledTracks.resize(35);
    std::cout << "[DiskDrive] Nibblizing 35 Tracks into GCR format..." << std::endl;
    for (int t = 0; t < 35; ++t) {
        nibblizeTrack(t, nibbledTracks[t].bytes);
    }
    std::cout << "[DiskDrive] Nibblization Complete!" << std::endl;
}

void DiskDrive::nibblizeTrack(int track, uint8_t* outBuffer) {
    size_t ptr = 0;
    
    auto rev2 = [](uint8_t b) {
        return ((b & 1) << 1) | ((b & 2) >> 1);
    };

    for (int sector = 0; sector < 16; ++sector) {
        // 1. Gap 1 / Sync: 20 bytes of 0xFF
        for (int i = 0; i < 20; ++i) outBuffer[ptr++] = 0xFF;

        // 2. Address Field Prologue
        outBuffer[ptr++] = 0xD5;
        outBuffer[ptr++] = 0xAA;
        outBuffer[ptr++] = 0x96;

        // 3. Address Header (Volume=254, Track, Sector, Checksum)
        uint8_t volume = 0xFE;
        uint8_t checksum = volume ^ track ^ sector;

        outBuffer[ptr++] = (volume >> 1) | 0xAA;
        outBuffer[ptr++] = volume | 0xAA;
        outBuffer[ptr++] = (track >> 1) | 0xAA;
        outBuffer[ptr++] = track | 0xAA;
        outBuffer[ptr++] = (sector >> 1) | 0xAA;
        outBuffer[ptr++] = sector | 0xAA;
        outBuffer[ptr++] = (checksum >> 1) | 0xAA;
        outBuffer[ptr++] = checksum | 0xAA;

        // 4. Address Field Epilogue
        outBuffer[ptr++] = 0xDE;
        outBuffer[ptr++] = 0xAA;
        outBuffer[ptr++] = 0xEB;

        // 5. Gap 2 / Sync: 10 bytes of 0xFF
        for (int i = 0; i < 10; ++i) outBuffer[ptr++] = 0xFF;

        // 6. Data Field Prologue
        outBuffer[ptr++] = 0xD5;
        outBuffer[ptr++] = 0xAA;
        outBuffer[ptr++] = 0xAD;

        // 7. 6-and-2 Data Encoding
        int logicalSector = DOS_PHYSICAL_TO_LOGICAL[sector];
        size_t fileOffset = (track * 16 + logicalSector) * 256;
        const uint8_t* rawData = &diskData[fileOffset];

        uint8_t encoded[342];
        // Pack 2-bit parts (6-and-2 GCR format)
        // D[i] 2-bit part is paired with G[i] bits 0-1
        // D[i+86] 2-bit part is paired with G[i] bits 2-3
        // D[i+172] 2-bit part is paired with G[i] bits 4-5
        for (int i = 0; i < 86; ++i) {
            uint8_t d0 = rawData[i] & 3;
            uint8_t d1 = (i + 86 < 256) ? (rawData[i + 86] & 3) : 0;
            uint8_t d2 = (i + 172 < 256) ? (rawData[i + 172] & 3) : 0;
            encoded[i] = rev2(d0) | (rev2(d1) << 2) | (rev2(d2) << 4);
        }
        // Pack 6-bit parts
        for (int i = 0; i < 256; ++i) {
            encoded[86 + i] = rawData[i] >> 2;
        }

        uint8_t xored[343];
        uint8_t last = 0; // Standard GCR has no sector bias
        for (int i = 0; i < 342; ++i) {
            uint8_t val = encoded[i];
            xored[i] = val ^ last;
            last = val;
        }
        xored[342] = last; // Checksum byte (no sector bias)

        // Map to 8-bit GCR bytes using lookup table
        for (int i = 0; i < 343; ++i) {
            outBuffer[ptr++] = GCR_WRITE_TABLE[xored[i]];
        }

        // 8. Data Field Epilogue
        outBuffer[ptr++] = 0xDE;
        outBuffer[ptr++] = 0xAA;
        outBuffer[ptr++] = 0xEB;
    }

    // Pad remaining space with Gap 3 Sync (0xFF)
    while (ptr < 6400) {
        outBuffer[ptr++] = 0xFF;
    }
}
