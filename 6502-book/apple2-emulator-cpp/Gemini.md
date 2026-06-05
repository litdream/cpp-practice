# Apple II+ Emulator (ver6-vibe) Project Plan

This project aims to create a working Apple II+ emulator in C++, referencing the `linapple` codebase and learning from the failures of `ver5-vibe`.

## Goals
- Functional 6502 CPU emulation.
- Accurate Apple II+ memory mapping (including I/O space).
- Video display emulation (Text, LGR, HGR).
- Keyboard input.
- Disk II emulation (loading .dsk images).
- Sound support (Speaker toggling).

## Proposed Architecture (Remake ver6-vibe)
We will aim for a more modular, object-oriented design than typical older emulators:

- **`CPU6502` Class**: Encapsulates registers, flags, and instruction decoding. Decoupled from memory via a `Bus` interface.
- **`Memory` Class**: Handles the 64KB address space, including bank switching (Language Card) and ROM mapping.
- **`IOController` Class**: Manages the $C000-$C0FF space, routing reads/writes to specific devices (Keyboard, Video switches, Disk II).
- **`Video` Class**: Emulates the display generator (Text, HGR, LGR) and renders to an SDL2 surface.
- **`DiskII` Class**: Emulates the Wozniak disk controller state machine.
- **`Bus` Interface**: Connects CPU to Memory and I/O.

---

## Phase 1: Foundation (CPU & Memory)
- [ ] Implement/Verify 6502 CPU core (Class structure).
- [ ] Implement Memory map (48K RAM, 12K ROM, I/O space).
- [ ] Load Apple II+ ROMs (Autostart ROM).
- [ ] Implement basic debug interface (disassembler, register viewer).

## Phase 2: Basic I/O & Display
- [ ] Implement Text mode video display (using SDL2).
- [ ] Implement Keyboard input ($C000 latch).
- [ ] Implement Speaker tick toggling ($C030).

## Phase 3: Advanced Video
- [ ] Implement Low-Resolution Graphics (LGR).
- [ ] Implement High-Resolution Graphics (HGR).
- [ ] Support mixed modes.

## Phase 4: Disk II & Peripherals
- [ ] Analyze `linapple` Disk II implementation.
- [ ] Implement Disk II controller state machine.
- [ ] Support loading `.dsk` (WOZ/NIB format support TBD).

## Phase 5: Polish & Compatibility
- [ ] Pass 6502 functional tests.
- [ ] Test with standard Apple II games (e.g., Snakebyte).
- [ ] Performance optimization.

---
*Note: This plan is iterative. Phase 1 is the immediate priority.*
