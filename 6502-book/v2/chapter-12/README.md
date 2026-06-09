# Chapter 12: Low-Resolution Graphics Mode (GR) & Mixed Mode

## 📖 Overview & Educational Philosophy
Welcome to Chapter 12! In this chapter, we expand our presentation layer by emulating the Apple II's **Low-Resolution (GR) Graphics Mode**. 

Up until now, our emulator has only been capable of rendering text. But a true 1977 computing experience is defined by its ability to display color graphics. We will implement the discrete hardware logic that translates memory into colored blocks instead of character shapes.

In this chapter, we:
1. **Intercept Video Soft Switches (`$C050`-`$C057`)**: Emulate the hardware switches that toggle between Text/Graphics, Full/Mixed screen, and Lores/Hires modes.
2. **Implement LORES Palette**: Define the classic 16-color Apple II palette.
3. **Deconstruct Video Memory (LORES Mapping)**: Map the 40x48 LORES grid from the same `$0400`-`$07FF` memory space used by Text Page 1.
4. **Build Mixed-Mode Rendering**: Implement "Mixed Mode" which displays 20 rows of graphics (40x40 blocks) on top and 4 rows of text at the bottom.

## 🛠️ The Hardware Dance: Soft Switches and Shared Memory
The Apple II does not have separate memory for text and graphics. Both Text Page 1 and LORES Page 1 share the exact same physical RAM range: `$0400` to `$07FF`. 

How does the monitor know whether to display text or colored blocks? It depends on the state of physical flip-flops in the video generation circuitry. These flip-flops are controlled by accessing **soft switches** (memory-mapped registers at `$C050`-`$C057`). 

When Graphics mode is active, each byte in `$0400`-`$07FF` is interpreted as two vertically stacked LORES blocks:
- The lower 4 bits (nibble) determine the color of the top block (0-15).
- The upper 4 bits determine the color of the bottom block (0-15).

In **Mixed Mode** (activated by `$C053`), the video generator dynamically switches its interpretation: rows 0-19 are rendered as graphics, while rows 20-23 are rendered as text. This allows games to show a graphical playfield while displaying scores or text prompts at the bottom of the screen.

## ⚙️ Building and Running Chapter 12
The emulator will boot into the classic Applesoft BASIC screen (Text mode). If you run a BASIC program that executes `GR`, the screen will switch to Low-Res graphics with the text window at the bottom!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-12
```
