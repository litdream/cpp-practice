# Apple II+ Boot Plan (To BASIC Prompt)

This document outlines the minimum requirements to boot the `ver6-vibe` emulator to the Applesoft BASIC prompt (e.g., when no disk drive is present).

## Requirements

To see the famous `APPLE ][` banner and the `]` prompt with a blinking cursor, we need to implement the following minimal components:

### 1. Memory Mapping & ROM
- **Main ROM ($D000 - $FFFF)**: 12KB of Apple II+ firmware (Applesoft BASIC + Autostart Monitor).
    - *Reference*: Linapple embeds `Apple2plus_rom` in `inc/resource.h`. We can use a similar approach or load it from a file.
- **RAM ($0000 - $BFFF)**: 48KB of standard RAM.
- **I/O Space ($C000 - $CFFF)**: Soft switches and hardware registers.
    - Specifically need access to Keyboard ($C000, $C010) and Video switches ($C050-$C057).

### 2. CPU Entry Point (Reset)
- The 6502 CPU must read the **Reset Vector** at `$FFFC - $FFFD` on startup.
- This vector points to the Autostart Monitor initialization code (usually `$FAA6` or similar in standard ROMs).

### 3. Text Mode Display
- **Text Buffer ($0400 - $07FF)**: 24 lines x 40 columns.
- **Interleaved Address Mapping**: Apple II text screen memory is not linear.
    - We should use a lookup table for the base address of each row:
      ```cpp
      const uint16_t ROW_BASE[] = {
          0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780,
          0x0428, 0x04A8, 0x0528, 0x05A8, 0x0628, 0x06A8, 0x0728, 0x07A8,
          0x0450, 0x04D0, 0x0550, 0x05D0, 0x0650, 0x06D0, 0x0750, 0x07D0
      };
      ```
- **Character Generator**: Map Apple II character codes (Normal, Inverse, Flashing) to displayable glyphs (using SDL2).

### 4. Keyboard Input (Minimal)
- **$C000 (Read)**: Returns the last pressed key code. Bit 7 is set if a new key is available.
- **$C010 (Read/Write)**: Clears the keyboard strobe (clears bit 7 of $C000).

---

## Boot Sequence Logic

1. **Initialize Memory**: Zero RAM, load ROM at `$D000-$FFFF`.
2. **Reset CPU**: Set PC to value at `$FFFC`.
3. **Run Loop**:
    * Execute 6502 instruction.
    * If PC reaches an I/O read/write, simulate the hardware response.
4. **Video Update**: Periodically (e.g., 60Hz) scan `$0400-$07FF` using the lookup table and update the SDL2 window.
5. **Autostart Scan**:
    * The ROM will scan slots for bootable cards.
    * If no Disk II card signature is found (empty slot space), the ROM will fall back to the BASIC prompt.

## Immediate Action Items (Remaking ver6-vibe)

- [ ] Extract or verify Apple II+ ROM bytes (12KB).
- [ ] Implement `Memory` class with `$D000-$FFFF` ROM mapping.
- [ ] Implement `CPU` class supporting basic instructions to run ROM code.
- [ ] Implement `Video` class that renders `$0400-$07FF` text buffer.
- [ ] Connect them via a simple `Bus`.

---
*Note: We do NOT need Disk II controller emulation ($C600) to reach the BASIC prompt.*
