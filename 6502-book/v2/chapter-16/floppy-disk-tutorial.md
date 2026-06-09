# Chapter 16 Tutorial: Deep Dive into Floppy Disk Emulation

Welcome to the definitive guide on how the Apple II Disk II floppy disk subsystem works, and how we emulate it in native C++ for Chapter 16.

This tutorial is divided into two parts:
1.  **The Hardware Perspective:** How Steve Wozniak's legendary Disk II controller and drive function at the physical and logical level.
2.  **The Software Implementation:** A line-by-line, commentary-style walkthrough of all the floppy disk related code in our emulator.

---

# Part 1: How the Apple II Disk II Works

The Disk II, released in 1978, is considered one of Steve Wozniak's greatest engineering triumphs. While contemporary floppy controllers used dozens of integrated circuits to handle disk formatting, track seeking, and byte synchronization in hardware, Wozniak realized he could eliminate almost all of them by shifting the complexity into **software (6502 machine code)**.

The hardware is extremely minimalist: a stepper motor, a spindle motor, a read/write head, a shift register, and a PROM containing a state machine (the Logic State Sequencer, or LSS). Everything else—from moving the head to decoding bits into bytes—is done by the CPU.

```
       +---------------------------------------------------+
       |                  APPLE II CPU                     |
       +------------------------+--------------------------+
                                | MMIO ($C0E0-$C0EF)
                                v
       +---------------------------------------------------+
       |             DISK II CONTROLLER CARD               |
       |  - Slot 6 ROM ($C600-$C6FF)                       |
       |  - Logic State Sequencer (LSS State Machine)       |
       |  - 8-bit Shift Register & Data Latch ($C0EC)       |
       +------------------------+--------------------------+
                                | Ribbon Cable
                                v
       +---------------------------------------------------+
       |                  DISK II DRIVE                    |
       |  - Spindle Motor (300 RPM)                        |
       |  - Stepper Motor (4 magnets, half-tracks 0-79)     |
       |  - Read/Write Head (Magnetic induction loop)      |
       +---------------------------------------------------+
```

### 1.1 Physical Layout of a Standard Disk (.DSK)
A standard 5.25-inch Apple II floppy disk contains:
*   **35 Tracks** (numbered 0 to 34).
*   **16 Sectors** per track.
*   **256 Bytes** of user data per sector.
*   **Total Capacity:** $35 \times 16 \times 256 = 143,360$ bytes (exactly 140 KB).

### 1.2 The Stepper Motor & Head Positioning
The drive head is moved inward and outward by a stepper motor. Unlike modern drives that automatically seek to a track, the Disk II controller exposes 4 electromagnets (phases 0 to 3) directly to the CPU via memory-mapped I/O switches:
*   `$C0E0` / `$C0E1` controls Phase 0 (OFF / ON)
*   `$C0E2` / `$C0E3` controls Phase 1 (OFF / ON)
*   `$C0E4` / `$C0E5` controls Phase 2 (OFF / ON)
*   `$C0E6` / `$C0E7` controls Phase 3 (OFF / ON)

By turning these magnets ON and OFF in a specific sequence, the CPU pulls the magnetic rotor, moving the head. The head can stop at **half-tracks** (0 to 79), allowing for extremely fine positioning (though standard DOS only uses even half-tracks, corresponding to the 35 logical tracks).

The physical movement is driven by magnetic pull. If the head is at track $N$ (aligned with phase $N \pmod 4$), turning on phase $(N+1) \pmod 4$ will pull the head forward by half a track. Turning on phase $(N-1) \pmod 4$ will pull it backward.

### 1.3 The Spindle Motor & Disk Rotation
The drive spindle motor spins the magnetic diskette at **300 RPM** (Rotations Per Minute), which means one full rotation takes exactly **200 milliseconds** ($200,000$ microseconds).
As the disk spins, the read head passes over a circular track. Data is written magnetically as a continuous stream of flux transitions (bits). 

At standard recording density, a single track can hold approximately **6,400 bytes** of raw, formatted data (including headers, gaps, and sync bytes).
*   $200,000 \text{ microseconds} / 6,400 \text{ bytes} \approx 31.25 \text{ microseconds per byte}$.
*   At the Apple II's 1.023 MHz clock rate, 1 microsecond $\approx 1$ CPU cycle.
*   Therefore, a new byte passes under the read head approximately every **32 clock cycles**.

### 1.4 The Shift Register & Latch ($C0EC)
As the magnetic transitions pass under the head, the controller card shifts the bits into an 8-bit shift register. 
*   **Bit Synchronization:** The controller relies on the fact that the most significant bit (MSB, Bit 7) of a valid disk byte is **always 1**.
*   When a `1` bit is shifted into the MSB position, the controller assumes a full byte has been assembled. It latches this byte into the data register (`$C0EC`) and raises a "byte ready" flag (which is simply keeping Bit 7 high).
*   The CPU reads the latch at `$C0EC`. If Bit 7 is `1`, a byte is ready. Reading the latch clears the "ready" state until the next byte is shifted in.

### 1.5 Why We Need GCR (Group Code Recording)
We cannot write raw 8-bit binary data directly to a magnetic disk. 
1.  **Magnetic Induction Limitations:** Magnetic heads detect *transitions* (change in magnetic polarity). If we write a long sequence of `0` bits (no transitions), the read head's clock will drift, and we will lose synchronization.
2.  **The Constraint:** To prevent drift, we must ensure that we never write more than two consecutive `0` bits, and the MSB (Bit 7) must always be `1` for byte synchronization.
3.  **The Solution (6-and-2 GCR):** Wozniak designed **Group Code Recording (GCR)**. We translate every 6 bits of user data into an 8-bit "disk byte" that satisfies these constraints. Out of 256 possible 8-bit values, only **64 values** meet the GCR constraints (MSB is 1, no more than two consecutive 0s, etc.). These 64 bytes are stored in a lookup table.

### 1.6 6-and-2 Encoding Process
To write a 256-byte sector, we must pack it into 6-bit chunks.
1.  **Split:** We take the 256 8-bit bytes and split them into:
    *   256 6-bit parts (the upper 6 bits of each byte).
    *   256 2-bit parts (the lower 2 bits of each byte).
2.  **Pack:** We pack the 2-bit parts into 86 bytes (since $86 \times 3 = 258$ bits, enough for 256 2-bit parts).
3.  **Combine:** This gives us $86 + 256 = 342$ bytes, all of which are 6-bit values (range 0 to 63).
4.  **XOR Checksum:** To detect errors, we XOR each byte with the previous one, creating a 343rd checksum byte.
5.  **Map:** We use the GCR lookup table to map these 343 6-bit values into 343 8-bit GCR disk bytes.

### 1.7 Track Format (Nibblization)
A raw track is not just a sequence of sectors. It contains structural framing to help the controller locate sectors:

```
+-----------------------------------------------------------------------------+
|                                  ONE TRACK                                  |
+-----------------------------------------------------------------------------+
| GAP 1 (48x $FF) | SECTOR 0 | GAP 3 (27x $FF) | SECTOR 1 | ... | GAP 3 | PAD |
+-----------------+----------+-----------------+----------+-----+-------+-----+

                 +--------------------------------------------+
                 |                 ONE SECTOR                 |
                 +--------------------------------------------+
                 | ADDRESS FIELD             | DATA FIELD     |
                 +---------------------------+----------------+
                 | Prologue:  $D5 $AA $96    | Prologue:  $D5 $AA $AD
                 | Volume:    4-and-4 encoded| Data:      342 GCR bytes
                 | Track:     4-and-4 encoded| Checksum:  1 GCR byte
                 | Sector:    4-and-4 encoded| Epilogue:  $DE $AA $EB
                 | Checksum:  4-and-4 encoded|
                 | Epilogue:  $DE $AA $EB    |
                 +---------------------------+----------------+
```

*   **Address Field:** Contains the metadata (Volume, Track, Sector numbers) so the CPU knows where the head is.
    *   Uses a simpler **4-and-4 encoding** (every 8-bit byte is split into two 4-bit nibbles, ORed with `$AA` to ensure MSB is 1) because the CPU needs to read this quickly without heavy decoding tables.
*   **Data Field:** Contains the 342 GCR bytes and the checksum.
*   **Prologues/Epilogues:** Unique byte signatures (`D5 AA 96`, `D5 AA AD`, `DE AA EB`) that act as "markers" so the CPU can align itself to the start of fields.
*   **Sync Gaps:** Blocks of `$FF` bytes written with extra clock transitions (10-bit sync bytes in hardware) that force the controller's shift register to align to byte boundaries.

---

# Part 2: Chapter 16 Code Walkthrough

Now let's examine how this hardware architecture is emulated in our Chapter 16 C++ code.

## 2.1 `SystemBus.cpp` (The Backplane)
The system bus intercepts Memory-Mapped I/O (MMIO) reads and writes in the `$C0E0` to `$C0EF` range (Slot 6, Disk II) and routes them to the `DiskDrive` instance.

[SystemBus.cpp:L48-54](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/SystemBus.cpp#L48-L54)
```cpp
    // 5. $C0E0-$C0EF: Disk II Slot 6 Soft Switches
    if (addr >= 0xC0E0 && addr <= 0xC0EF) {
        if (diskDrive) {
            return diskDrive->readSwitch(addr - 0xC0E0, getSystemCycles());
        }
        return 0x00;
    }
```
*   **Commentary:** Any CPU read to `$C0E0-$C0EF` is passed to the disk drive. We pass the relative address (`0` to `15`) and the current absolute `systemCycles` (crucial for timing-based disk rotation).

[SystemBus.cpp:L83-89](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/SystemBus.cpp#L83-L89)
```cpp
    // 4. $C0E0-$C0EF: Disk II Slot 6 Soft Switches
    if (addr >= 0xC0E0 && addr <= 0xC0EF) {
        if (diskDrive) {
            diskDrive->writeSwitch(addr - 0xC0E0, value);
        }
        return;
    }
```
*   **Commentary:** Writes are handled similarly. In physical hardware, accessing a switch (whether reading or writing) has the exact same electrical effect of toggling the latch/magnet.

---

## 2.2 `DiskDrive.h` (The Controller State)
The `DiskDrive` class encapsulates the state of the active floppy disk drive.

[DiskDrive.h:L25-33](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/inc/DiskDrive.h#L25-L33)
```cpp
    std::vector<uint8_t> diskData; // Raw .dsk file data (143,360 bytes)
    bool hasDisk = false;

    int currentHalfTrack = 0; // 0 to 79 (40 tracks * 2 half-tracks)
    bool motorOn = false;
    int magnetStates = 0; // Bitmask of active stepper phases (0-3)
    bool writeMode = false; // Q7 state
    bool shiftMode = false; // Q6 state
    int activeDrive = 1;
```
*   **Commentary:**
    *   `diskData`: Holds the raw 140KB sector-ordered image.
    *   `currentHalfTrack`: Tracks the physical head position (0 to 79).
    *   `magnetStates`: A 4-bit mask representing which stepper motor electromagnets are currently energized (ON/OFF).
    *   `writeMode` / `shiftMode`: Emulates the state of the sequencer controller (Q6/Q7 switches).
    *   `activeDrive`: Tracks which drive (1 or 2) is selected.

[DiskDrive.h:L36-39](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/inc/DiskDrive.h#L36-L39)
```cpp
    // Physical Track representation in GCR format
    struct NibbledTrack {
        uint8_t bytes[6400];
    };
    std::vector<NibbledTrack> nibbledTracks;
```
*   **Commentary:**
    *   `NibbledTrack`: A physical representation of a track containing 6,400 raw GCR bytes.
    *   `nibbledTracks`: Array of 35 tracks. We convert the logical DSK sectors into this GCR bitstream on startup to allow low-level emulation (LLE) reading.

---

## 2.3 `DiskDrive.cpp` (The Emulation Engine)

### 2.3.1 The Stepper Motor & Head Movement
This is the core of the physical seek emulation.

[DiskDrive.cpp:L53-85](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/DiskDrive.cpp#L53-L85)
```cpp
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
```
*   **Commentary:**
    *   `phase` / `state`: Decodes which phase (0-3) is targeted, and whether it is being turned ON (odd address) or OFF (even address).
    *   `magnetStates |= phase_bit` / `&= ~phase_bit`: Updates our 4-bit active magnet register.
    *   `direction` calculation:
        *   If the magnet at `(currentHalfTrack + 1) & 3` (one half-step ahead) is ON, it exerts a magnetic pull forward (`direction += 1`).
        *   If the magnet at `(currentHalfTrack + 3) & 3` (one half-step behind, same as `currentHalfTrack - 1`) is ON, it exerts a pull backward (`direction -= 1`).
        *   If both are ON, they cancel out, resulting in no movement.
        *   If the pull is non-zero, we call `moveHead` to physically slide the head.

[DiskDrive.cpp:L142-153](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/DiskDrive.cpp#L142-L153)
```cpp
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
```
*   **Commentary:** Moves the head by `steps` half-tracks, clamping it between 0 and 79 (physical limits). We calculate the logical track (`currentHalfTrack / 2`) and log a message only if the head has crossed a full track boundary, avoiding verbose logs during half-steps.

### 2.3.2 Motor & Sequencer Switch Control
The rest of `writeSwitch` handles the state switches.

[DiskDrive.cpp:L86-117](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/DiskDrive.cpp#L86-L117)
```cpp
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
```
*   **Commentary:** Standard mapping of addresses 8 through 15 to system parameters.
    *   `$C0EC` / `$C0ED` controls Q6 (Shift/Latch). In read mode, Q6L (Shift) tells the controller to shift bits in and present them on the data bus.
    *   `$C0EE` / `$C0EF` controls Q7 (Read/Write). Q7L is Read mode.

### 2.3.3 Timing-Based Bitstream Reading
This simulates the disk physically spinning under the head.

[DiskDrive.cpp:L120-155](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/DiskDrive.cpp#L120-L155)
```cpp
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
```
*   **Commentary:**
    *   `writeSwitch(addr, 0)`: Toggles Q6/Q7 state as a side-effect of reading.
    *   `writeMode` check: If Q7 is in Write mode (`$C0EF`) and we read `$C0EC` or `$C0ED`, the controller hardware returns the **Write Protect** status. We return `0x00` (meaning the disk is writable, not write-protected).
    *   `currentByteIndex` calculation:
        *   We divide total absolute `systemCycles` by `32` (cycles per byte) and modulo by `6,400` (bytes per track). This tells us exactly which byte is under the virtual read head at this microsecond.
    *   `byteReady` logic:
        *   If the calculated index has changed since the last poll (`currentByteIndex != lastByteIndex`), a new byte has spun under the head, so we set `byteReady = true`.
        *   When the CPU reads `$C0EC` (addr 12):
            *   If `byteReady` is true, we return the raw GCR byte (which has MSB=1). We set `byteReady = false` so subsequent extremely rapid reads of the same byte will show it is not ready.
            *   If `byteReady` is false (the disk hasn't spun to the next byte yet), we return `trackByte & 0x7F` (clearing the MSB to 0). This tells the CPU's `BPL` polling loop that the byte is not ready yet, forcing it to wait.

---

### 2.3.4 GCR 6-and-2 Encoder (Nibblization)
This transforms the flat sector-ordered DSK image into physical tracks on startup.

[DiskDrive.cpp:L177-214](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/DiskDrive.cpp#L177-L214)
```cpp
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
```
*   **Commentary:** We construct the track sector by sector (0 to 15).
    *   **Sync Gaps:** Write `0xFF` bytes to align the CPU.
    *   **Address Field:** We write the prologue `D5 AA 96`. Then we encode Volume (`0xFE`), Track, Sector, and checksum using **4-and-4 encoding** (each byte is written as two bytes: `(val >> 1) | 0xAA` and `val | 0xAA`, ensuring no long runs of zeros). Finally, we write the epilogue `DE AA EB`.
    *   **Data Field:** Begins with prologue `D5 AA AD`.

[DiskDrive.cpp:L218-247](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/DiskDrive.cpp#L218-L247)
```cpp
        // 7. 6-and-2 Data Encoding
        int logicalSector = DOS_PHYSICAL_TO_LOGICAL[sector];
        size_t fileOffset = (track * 16 + logicalSector) * 256;
        const uint8_t* rawData = &diskData[fileOffset];

        uint8_t encoded[342];
        // Pack 2-bit parts (6-and-2 GCR format)
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
```
*   **Commentary:** This is Wozniak's **6-and-2 encoding** algorithm.
    *   `DOS_PHYSICAL_TO_LOGICAL`: Translates physical sector index on the disk to the logical sector order of DOS 3.3.
    *   `rev2`: Reverses the bits of the 2-bit chunks.
    *   **Packing loop (`i < 86`):** We take the 2-bit parts of three bytes (`rawData[i]`, `rawData[i+86]`, `rawData[i+172]`), reverse them, and pack them into a single 6-bit byte `encoded[i]`. For the final iterations (84 and 85), `i+172` is out-of-bounds (since $84+172 = 256$), so `d2` defaults to `0` (the unused padding bits).
    *   **Unpacked loop (`i < 256`):** We copy the upper 6 bits of the 256 raw bytes into the remainder of the array (`encoded[86]` through `encoded[341]`).
    *   **XOR loop:** We XOR each byte with the previous one (`val ^ last`) and append the final running XOR value as a 343rd checksum byte.

[DiskDrive.cpp:L248-263](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/DiskDrive.cpp#L248-L263)
```cpp
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
```
*   **Commentary:**
    *   `GCR_WRITE_TABLE`: Maps the 343 6-bit values (range 0 to 63) to valid GCR 8-bit bytes (which always have Bit 7 set to 1 and obey the zero-run constraints).
    *   Epilogue `DE AA EB` is written.
    *   We pad the remainder of the 6,400-byte track buffer with `0xFF` (Gap 3) to fill the physical track size.

---

## 2.4 `CPU6502.cpp` (The HLE Boot Intercept)
Because we do not run the actual Slot 6 ROM assembly code to read raw tracks (which would be slow and require extremely precise 10-bit bit-shifting emulation), we intercept the execution at `$C600` (Slot 6 ROM entry point) and perform a **High-Level Emulation (HLE)** of the bootloader.

[CPU6502.cpp:L87-124](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/CPU6502.cpp#L87-L124)
```cpp
        // HLE Boot ROM Intercepts
        if (PC == 0xC600) {
            std::cout << "[HLE Boot] Intercepted Slot 6 Boot at $C600" << std::endl;
            if (disk) {
                // 1. Load Sector 0 to $0800-$08FF
                std::vector<uint8_t> sector0 = disk->readSector(0, 0);
                for (int i = 0; i < 256; ++i) {
                    bus->write(0x0800 + i, sector0[i]);
                }

                // 2. Populate GCR-to-6bit translation table in RAM at $0356-$03FF (base $02D6)
                // This mimics what the physical Slot 6 ROM bootloader routine does before jumping to $0801,
                // enabling the custom 6502 game loader's LLE GCR reader to successfully decode disk data!
                const uint8_t GCR_WRITE_TABLE[64] = { ... };
                for (int i = 0; i < 64; ++i) {
                    bus->write(0x02D6 + GCR_WRITE_TABLE[i], i);
                }
                std::cout << "[HLE Boot] Populated GCR-to-6bit translation table in RAM." << std::endl;
            } else {
                std::cerr << "[HLE Boot] Error: No virtual disk drive available!" << std::endl;
            }
            X = 0x60;
            Y = 0x00;
            bus->write(0x2B, 0x60);
            PC = 0x0801; 
            continue; 
        }
```
*   **Commentary:**
    *   **Sector 0 Load:** We read Track 0, Sector 0 directly from our DSK image using `readSector(0, 0)` and write those 256 bytes directly to RAM starting at `$0800` (where the Apple II boots programs).
    *   **GCR Translation Table Population:** This is a vital step. The physical Slot 6 ROM bootloader contains an initialization routine that constructs a translation table in RAM at `$0356-$03FF` (using the base address `$02D6`). This table is used by the game loader's own low-level disk routines. Since we bypassed the ROM bootloader, we **must** manually populate this table in RAM so the game loader doesn't crash when trying to decode subsequent tracks!
    *   **Registers setup:** The physical bootloader leaves the Slot offset ($60 for Slot 6) in the `X` register and in RAM at `$2B`. The game loader relies on this to access the MMIO switches (e.g. `LDA $C08C,X` -> reads `$C0EC`). We manually set `X = 0x60`, `bus->write(0x2B, 0x60)`, and jump the program counter `PC` directly to `$0801` to let the game's loader take over.

---

## 2.5 `main.cpp` (The Bootstrapper)
Finally, we must convince the Apple II's Autostart ROM that a Disk II controller is installed in Slot 6 during a cold reset.

[main.cpp:L41-49](file:///usr/local/google/home/raychung/prg/cpp-practice/6502-book/v2/chapter-16/src/main.cpp#L41-L49)
```cpp
    // 4. Set up Slot 6 ROM Boot Signatures in virtual Memory
    // These signatures are read by the Apple II Autostart ROM reset scan routine
    // to detect that a bootable disk controller is present in Slot 6.
    memory.writeSlotROM(0xC601, 0x20); // LDX #$20 signature
    memory.writeSlotROM(0xC603, 0x00); // LDY #$00 signature
    memory.writeSlotROM(0xC605, 0x03); // LDX #$03 signature
    memory.writeSlotROM(0xC607, 0x3C); // Autostart ROM signature
    memory.writeSlotROM(0xC63C, 0x3C); // ProDOS slot-ROM signature
```
*   **Commentary:**
    *   During system reset, the Apple II Autostart ROM scans slot ROMs backward from Slot 7 to Slot 1.
    *   To identify a bootable Disk II controller in Slot 6, the ROM checks for specific byte signatures at hardcoded offsets in the slot's address space (`$C600` range).
    *   We write these exact signatures (`0x20`, `0x00`, `0x03`, `0x3C`) to virtual Slot 6 memory.
    *   When we call `cpu.reset()`, the Autostart ROM will detect these signatures, identify Slot 6 as bootable, and jump to `$C600`, triggering our HLE bootloader intercept!

---

## Summary of the Emulation Loop

```
[main.cpp]               [SystemBus]             [CPU6502]               [DiskDrive]
    |                         |                      |                        |
    +---- cpu.reset() ------->|                      |                        |
    |                         |-- (Scan Slot 6 ROM)->|                        |
    |                         |                      |-- (PC == $C600 HLE) ->|
    |                         |                      |                        |-- readSector(0,0) -> $0800
    |                         |                      |                        |-- Build GCR Table in RAM
    |                         |                      |-- (JMP to $0801)------>|
    |                         |                      |                        |
    |<--- [Loop 60 FPS] ------|                      |                        |
    |                         |                      |                        |
    |-- cpu.execute(17050) -->|                      |                        |
    |                         |-- (LDA $C0EC,X)----->|                        |
    |                         |   [addr = 12]        |-- readSwitch(12)------>|
    |                         |                      |                        |-- Calculate track byte index
    |                         |                      |<-- returns GCR byte ---|
    |                         |<-- returns to CPU ---|                        |
    |                         |                      |                        |
    |                         |-- (LDA $C081,X)----->| (Pulsing Stepper)      |
    |                         |   [addr = 1]         |-- writeSwitch(1)------>|
    |                         |                      |                        |-- Update magnet states
    |                         |                      |                        |-- Move head (direction)
    |                         |                      |                        |
    |-- (Limit Frame Time) -->|                      |                        |
```

This elegant chain of hardware-software interaction is what allows the emulator to boot real floppy disks like `SNAKEBYTE.DSK` at physical-level accuracy, rendering the graphics frame-by-frame on your modern SDL2 screen!
