# Chapter 14: High-Resolution Graphics Page 2 (HGR2) & Soft Switch Display Routing

## 📖 Overview & Educational Philosophy
Welcome to Chapter 14! Now that we have successfully emulated all basic display modes (Text, Lores, Hires Page 1, and Mixed mode), we take the next logical architectural step: **emulating High-Resolution Graphics Page 2 (HGR2)** and implementing comprehensive **Page 2 Soft Switch Display Routing**.

In computer architecture, graphics pages allow developers to implement techniques like **double buffering** (drawing to an off-screen page while displaying the on-screen page to avoid tearing). On the Apple II, the system features second pages for both Text/Lores (`$0800-$0BFF`) and High-Resolution Graphics (`$4000-$5FFF`).

In this chapter, we:
1. **Model Page 2 Memory Map**: Understand the physical memory layout where Text/Lores Page 2 is mapped to `$0800-$0BFF` and HGR Page 2 is mapped to `$4000-$5FFF`.
2. **Implement Dynamic Page Routing**: Modify our `Video` rendering subsystem to dynamically read from either Page 1 or Page 2 depending on the state of the `$C054`/`$C055` soft switches.
3. **Build an Interactive Soft-Switch Playground**: Create a C++ test program that pre-populates all display pages with distinct visual patterns and provides keyboard controls to interactively toggle all video modes and page selections.

## 🛠️ The Hardware Dance: Page 2 & Soft Switches

On physical hardware, the display generator fetches bytes from memory and outputs signals to the screen. By default, it reads from Page 1. When the CPU reads or writes to memory-mapped I/O (MMIO) soft switches, it toggles internal flip-flops that control which memory ranges are queried:

| Soft Switch | Address | Description |
| :--- | :--- | :--- |
| **TEXT** | `$C051` | Select Text display mode |
| **GRAPHICS** | `$C050` | Select Graphics display mode (Lores or Hires) |
| **FULL** | `$C052` | Select Full Screen graphics (192 rows) |
| **MIXED** | `$C053` | Select Mixed Mode (160 rows graphics + 4 rows text) |
| **PAGE 1** | `$C054` | Select Page 1 display memory (`$0400` Text, `$2000` HGR) |
| **PAGE 2** | `$C055` | Select Page 2 display memory (`$0800` Text, `$4000` HGR) |
| **LORES** | `$C056` | Select Low-Resolution graphics |
| **HIRES** | `$C057` | Select High-Resolution graphics |

In this chapter, we update `Video.cpp` so that:
- **Text/Lores rendering**: Queries `$0400` (Page 1) or `$0800` (Page 2) by applying a dynamic `0x0400` offset when `bus->isPage2Mode()` is true.
- **Hires rendering**: Obtains scanline row base addresses starting at `$2000` (HGR1) or `$4000` (HGR2) dynamically using:
  ```cpp
  uint16_t base = bus->isPage2Mode() ? 0x4000 : 0x2000;
  ```

## ⚙️ Building and Running Chapter 14

This chapter compiles into an interactive, visual deconstruction dashboard. Instead of booting directly into a silent Applesoft prompt, we pre-fill Page 1 and Page 2 with distinct patterns so you can immediately see the effect of the hardware soft switches.

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-14
```

### Interactive Controls:
Press the following keys on your keyboard to toggle the virtual computer's physical soft switches in real-time:
* **`[1]`** - Select **Page 1** (Displays HGR Page 1 & Text Page 1)
* **`[2]`** - Select **Page 2** (Displays HGR Page 2 & Text Page 2)
* **`[G]`** - Set to **Graphics Mode**
* **`[T]`** - Set to **Text Mode**
* **`[M]`** - Set to **Mixed Mode** (Graphics with 4 lines of text at the bottom)
* **`[F]`** - Set to **Full Screen** (Graphics only, no text)
* **`[H]`** - Set to **Hires Graphics**
* **`[L]`** - Set to **Lores Graphics**
* **`[ESC]`** - Quit the emulator

