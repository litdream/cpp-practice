# Chapter 15: Pristine Apple II Emulator (Custom Program Playground)

## 📖 Overview & Educational Philosophy
Welcome to Chapter 15! In this chapter, we assemble a **Pristine Apple II Emulator** featuring all the deconstructed hardware subsystems we've built throughout this book—including Text, Lores, Speaker Sound, HGR1, and the recently implemented HGR2/Page 2 Soft Switches.

Unlike previous chapters which executed synthetic test harnesses or pre-populated memory from C++, Chapter 15 boots directly into the original emulated ROM environment (Applesoft BASIC / System Monitor). This gives you a clean slate to write and run your own 6502 programs, allowing you to test the HGR2 and Page 2 display switches using native 6502 instructions!

In this chapter, we:
1. **Restore Full Keyboard Strobe Routing**: Map your keyboard entries directly to the emulated Apple II keyboard strobe buffer so you can type and write BASIC or assembly code inside the emulated machine.
2. **Boot to Native ROM**: Execute the MOS 6502 engine starting from its cold reset vector, landing on the legendary `Apple ][` BASIC prompt.
3. **Pristine State Maintenance**: Remove all C++ diagnostic overrides and visual pre-populators, leaving video memory (`$0400-$0BFF` and `$2000-$5FFF`) entirely under the control of the emulated CPU.

## 🛠️ Writing Custom Programs for HGR2 Test

Since HGR2 is now fully modeled in the system bus and video routing, you can test it directly by writing a 6502 machine program or Applesoft BASIC code.

### Option A: Testing HGR2 via Applesoft BASIC
Applesoft BASIC has built-in support for HGR (Page 1), but you can manipulate the Page 2 soft switch using `PEEK` and `POKE`.
For example, to display HGR Page 2:
1. Initialize Hires graphics mode (this usually defaults to HGR Page 1):
   ```basic
   HGR
   ```
2. Toggle the soft switch at `$C055` (decimal `49237`) to select **Page 2** instead of Page 1:
   ```basic
   POKE 49237, 0
   ```
   Now the screen displays HGR Page 2! Since it is empty, the top graphics area will go black.
3. Draw a line in BASIC:
   Wait, the BASIC `HPLOT` command always draws to the "active" BASIC graphics page (which BASIC thinks is Page 1). But by poking `$C055` you are viewing Page 2.
   To actually draw to Page 2, you can write to the memory range `$4000-$5FFF`.

### Option B: Testing HGR2 via System Monitor (6502 Assembly)
You can enter the Apple II System Monitor by typing `CALL -151` in BASIC.
Then, you can write a short machine code program at `$0300` to fill HGR Page 2 (`$4000-$5FFF`) and toggle Page 2:

1. Enter Monitor:
   ```basic
   CALL -151
   ```
2. Write assembly instructions to fill HGR2 with a pattern and display it:
   ```monitor
   0300: A9 55    ; LDA #$55 (violet/green stripes pattern)
   0302: A2 40    ; LDX #$40 (start page of HGR2)
   0304: A0 00    ; LDY #$00 (offset)
   0306: 91 00    ; STA ($00),Y (using zero page pointer, assuming $00-$01 contains $00 and $40)
   ...
   ```
   Alternatively, you can just manually write bytes to `$4000` inside the monitor to see them appear on screen when Page 2 is displayed:
   * Select Hires Page 2: type `C050` (Graphics), `C053` (Mixed), `C057` (Hires), `C055` (Page 2) by reading them:
     ```monitor
     C050
     C053
     C057
     C055
     ```
   * Write to HGR2 memory (e.g. `$4000` and `$4001`):
     ```monitor
     4000: FF FF FF FF FF FF
     ```
     You will immediately see white horizontal segments appear at the top-left of the HGR2 screen!

## ⚙️ Building and Running Chapter 15

### Build Steps:
```bash
mkdir -p build
cmake -S . -B build
cmake --build build
./build/chapter-15
```

### Controls:
* **Keyboard**: Type normally. All lowercase keys are automatically capitalized to match Apple II specifications.
* **`[ESC]`**: Quit the emulator.
* **`[F1]`**: Print the current CPU register states (`PC`, `A`, `X`, `Y`, `S`, `P`) to the terminal.
* **`[F2]`**: Dump the first page (Zero-Page, `$0000-$00FF`) of memory to the terminal.
