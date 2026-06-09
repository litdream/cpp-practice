# Chapter 13: High-Resolution Graphics Mode (HGR) & Mixed Mode

## 📖 Overview & Educational Philosophy
Welcome to Chapter 13! In this chapter, we implement the third and most visually impressive display mode of the classic Apple II: **High-Resolution (HGR) Graphics Mode**.

HGR mode offers a resolution of 280x192 pixels with a unique 6-color palette. Emulating HGR is a true test of system architecture understanding, as it combines complex interleaved memory addressing with NTSC-like color artifact generation.

In this chapter, we:
1. **Deconstruct HGR Memory Mapping**: Implement the complex 3-level interleaved addressing formula to map 280x192 pixels from HGR Page 1 (8KB starting at `$2000`).
2. **Emulate Color Artifact Generation**: Implement Wozniak's ingenious color generation scheme, where colors are determined by even/odd pixel positions, adjacent dot states, and the high-bit (color-delay bit) of each byte.
3. **Integrate SDL Scaling**: Use a temporary 280x192 surface and `SDL_BlitScaled` to stretch the HGR screen to fit our 320x192 backbuffer, ensuring perfect alignment with text mode.
4. **Build HGR Mixed Mode**: Support displaying 160 rows of HGR graphics on top and 4 rows of text at the bottom.

## 🛠️ The Hardware Dance: NTSC Artifacts and Interleaving
Unlike modern framebuffers where each pixel has a direct color value, the Apple II HGR mode is monochrome at the hardware level. The colors you see on a TV screen are **NTSC artifacts** caused by the frequency of the pixel clock relative to the color subcarrier.

Each byte in HGR memory (`$2000` to `$3FFF`) represents 7 pixels:
- **Bits 0-6**: Represent the 7 pixels (left-to-right, bit 0 is leftmost).
- **Bit 7 (High Bit)**: Shifts the phase of the pixels by half a pixel width. This changes the color palette for those 7 pixels from Palette 0 (Green/Violet) to Palette 1 (Orange/Blue).

The color of an active dot (1) is determined by its horizontal position:
- **Even columns**: Violet (Palette 0) or Blue (Palette 1).
- **Odd columns**: Green (Palette 0) or Orange (Palette 1).
- **Adjacent active dots**: Merge to form solid **White**.
- **Inactive dots (0)**: Render as **Black**.

Furthermore, HGR memory is highly interleaved to optimize video hardware refresh cycles. The address of scanline $Y$ is calculated using a complex formula involving blocks, groups, and lines, which we resolve in `Video::getHgrRowAddr(y)`.

## ⚙️ Building and Running Chapter 13
The emulator will boot into the classic Applesoft BASIC screen (Text mode). If you run a BASIC program that executes `HGR`, the screen will switch to High-Res graphics with the text window at the bottom!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-13
```
