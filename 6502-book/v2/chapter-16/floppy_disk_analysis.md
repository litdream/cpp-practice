# Floppy Disk Emulation Analysis: AppleWin vs. linapple vs. Our Implementation

This document analyzes the floppy disk emulation in **AppleWin** and **linapple** (which is a Linux port of AppleWin), focusing on why our Chapter 16 implementation fails to complete loading games like `SNAKEBYTE.DSK` (hanging in the middle of loading HGR2 graphics).

---

## 1. The Symptom & Diagnosis

*   **Symptom:** The emulator boots from Slot 6 (via HLE), starts loading, displays a few HGR2 graphic lines, and then hangs.
*   **Diagnosis:** The fact that it starts loading and displays some graphics means:
    1.  The HLE bootloader successfully loaded Sector 0.
    2.  The game's custom loader (LLE) started running and successfully read some sectors (likely from Track 0).
    3.  The hang occurs when the loader attempts to seek to Track 1 (or subsequent tracks) and read data.
*   **Root Cause:** Our current stepper motor emulation is too simplistic and fails to move the head if the loader uses **single-phase stepping** (turning off the previous phase before turning on the next phase), which is common in custom fast-loaders.

---

## 2. Stepper Motor Emulation Comparison

The Disk II drive uses a stepper motor with 4 phases (electromagnets 0-3) to move the read/write head. The head position is measured in half-tracks (0 to 79).

### Our Current Implementation (`v2/chapter-16`)
We only move the head on active transitions (turning a phase ON) and require the *previous* phase to still be ON at that exact moment:

```cpp
// DiskDrive.cpp
if (state && !oldState) { // Only on transition to ON
    int prevPhase = (phase - 1) & 3;
    int nextPhase = (phase + 1) & 3;

    if (phaseStates[prevPhase]) {
        moveHead(1);  // Move inward
    } else if (phaseStates[nextPhase]) {
        moveHead(-1); // Move outward
    }
}
```

*   **Failure Case (Single-Phase Stepping):**
    1.  Phase 0 is ON (Head at Track 0).
    2.  Phase 0 is turned OFF (No move).
    3.  Phase 1 is turned ON. Since Phase 0 is now OFF, `phaseStates[prevPhase]` is false. **No move occurs.**
    4.  The head remains stuck at Track 0 forever.

### `linapple` & `AppleWin` Implementation
They track the *collective state* of all magnets as a bitmask (`magnetStates`) and evaluate the pull on the head based on which magnets are currently ON relative to the current head position, regardless of when they were turned ON/OFF.

```cpp
// linapple / Disk.cpp (simplified)
int direction = 0;
// If the magnet "one step ahead" of current phase is ON, pull forward
if (magnetStates & (1 << ((currentHalfTrack + 1) & 3))) {
    direction += 1;
}
// If the magnet "one step behind" of current phase is ON, pull backward
if (magnetStates & (1 << ((currentHalfTrack + 3) & 3))) {
    direction -= 1;
}

if (direction) {
    currentHalfTrack = clamp(0, 79, currentHalfTrack + direction);
}
```

*   **Why it works:**
    1.  Phase 0 is ON. `magnetStates` = `0001`. Head is at `0`.
    2.  Phase 0 is turned OFF. `magnetStates` = `0000`. No move.
    3.  Phase 1 is turned ON. `magnetStates` = `0010`. Head is still at `0`.
    4.  Evaluation:
        *   Adjacent magnet `(0 + 1) & 3 = 1` is ON -> `direction += 1`.
        *   Adjacent magnet `(0 + 3) & 3 = 3` is OFF.
        *   `direction` = `+1`. Head moves to `1` (half-track 1).
    This successfully moves the head even with zero overlap between phase activations!

---

## 3. Read/Write Timing & Latch Emulation

### Our Current Implementation
We use a purely time-based approach using `systemCycles` to determine which byte of the GCR track is under the head:

```cpp
uint64_t currentByteIndex = (systemCycles / 32) % 6400;
```

We return the byte if it's "ready" (index changed), otherwise we clear the MSB to signal "not ready". This is theoretically accurate but highly sensitive to CPU cycle precision and drift.

### `linapple` Implementation
`linapple` uses a hybrid approach:
*   **Enhanced Mode (Default):** The disk "spins" only when read. `DiskReadWrite` returns the next byte and increments the track byte pointer *every time it is called*, regardless of cycles. This is extremely robust for standard loaders because it feeds data as fast as the CPU can read it.
*   **Authentic Mode:** If `enhancedisk` is false, it updates the byte pointer based on cycles in a main loop hook, similar to our approach.

### `AppleWin` Implementation
AppleWin is extremely precise:
*   It operates at the **bit level** rather than byte level, shifting bits in based on cycle deltas.
*   It emulates the **Logic State Sequencer (LSS)** for WOZ images.
*   It handles **Write Protect (WP)** sensing by latching `0xFF` (if WP) or `0x00` (if not) when `$C0ED` (Q6H) is accessed, which is then read when `$C0EE` (Q7L, even address) is accessed.

---

## 4. Proposed Fix for Our Emulator

To fix the Chapter 16 hang, we should adopt the robust stepper motor logic from `linapple`/`AppleWin`. We can keep our time-based byte reading for now, as it is more "pedagogical" (showing how cycles map to disk rotation), but fixing the stepper motor is critical.

### Step 1: Update `DiskDrive.h`
We need to add `magnetStates` to track active phases.

```diff
class DiskDrive {
...
private:
    int currentHalfTrack = 0; // 0 to 79 (was 0 to 69)
    bool motorOn = false;
-   bool phaseStates[4] = {false, false, false, false};
+   int magnetStates = 0; // Bitmask of active stepper phases (0-3)
    bool writeMode = false; // Q7 state
...
};
```

### Step 2: Update `DiskDrive.cpp`
Rewrite `writeSwitch` stepper phase handling to use the magnet state bitmask and adjacent-pull logic.

```diff
void DiskDrive::writeSwitch(uint16_t addr, uint8_t value) {
    int phase = (addr >> 1) & 3;
    bool state = (addr & 1) != 0;

    if (addr >= 0 && addr <= 7) {
-       // Stepper motor phases (0 to 3)
-       bool oldState = phaseStates[phase];
-       phaseStates[phase] = state;
-
-       if (state && !oldState) {
-           int prevPhase = (phase - 1) & 3;
-           int nextPhase = (phase + 1) & 3;
-
-           if (phaseStates[prevPhase]) {
-               moveHead(1); // Step inward (higher track)
-           } else if (phaseStates[nextPhase]) {
-               moveHead(-1); // Step outward (lower track)
-           }
-       }
+       // Update the magnet states bitmask
+       int phase_bit = (1 << phase);
+       if (state) {
+           magnetStates |= phase_bit;  // phase on
+       } else {
+           magnetStates &= ~phase_bit; // phase off
+       }
+
+       // Evaluate stepping effect based on active magnets relative to current head position
+       int direction = 0;
+       if (magnetStates & (1 << ((currentHalfTrack + 1) & 3))) {
+           direction += 1;
+       }
+       if (magnetStates & (1 << ((currentHalfTrack + 3) & 3))) {
+           direction -= 1;
+       }
+
+       if (direction) {
+           moveHead(direction);
+       }
    } else {
        switch (addr) {
            case 8: // $C0E8: Motor OFF
```

And update `moveHead` to clamp to 79 (40 tracks of half-steps) and improve logging:

```diff
void DiskDrive::moveHead(int steps) {
+   int oldTrack = getCurrentTrack();
    currentHalfTrack += steps;
    if (currentHalfTrack < 0) currentHalfTrack = 0;
-   if (currentHalfTrack > 69) currentHalfTrack = 69;
-   std::cout << "[DiskDrive] Stepper Motor Active. Head moved to half-track " 
-             << currentHalfTrack << " (Track " << getCurrentTrack() << ")" << std::endl;
+   if (currentHalfTrack > 79) currentHalfTrack = 79;
+
+   int newTrack = getCurrentTrack();
+   if (newTrack != oldTrack) {
+       std::cout << "[DiskDrive] Stepper Motor Active. Head moved to half-track " 
-                 << currentHalfTrack << " (Track " << newTrack << ")" << std::endl;
+   }
}
```
