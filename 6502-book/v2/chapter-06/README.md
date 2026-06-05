# Chapter 6: Video Architecture & Font Rasterization

## 📖 Overview & Educational Philosophy
Welcome to Chapter 6! You have wired the copper backplane of the System Bus and synthesized the central clock and execution Topologies of the 6502 CPU. Now, it is time to build the **eye and display** of the Apple II: the **Discrete Logic Video Circuit**.

In this chapter, we emulate the structural parameters of the Apple II primary text page ($0400–$07FF), synthesize a software font renderer using production XPM character bitmaps, and learn to translate raw byte ranges in Virtual RAM into visible text on an SDL2 window.

## 🛠️ The Hardware Dance: Memory Mapping to Pixels
Operating at exactly 60Hz, the Apple II hardware fetches bytes continuously from internal RAM addresses $0400 through $07FF. Bits 7 and 6 determine the style (Normal, Inverse, or Flashing), while Bits 0-5 select one of 64 localized shape matrices.

## ⚙️ Building and Running Chapter 6
Our `main.cpp` initializes the video subsystem, injects the classic "HELLO WORLD" and Apple ][ styling into the Text RAM map, and actually displays the rendered emulated frame inside an OS window for 3 seconds!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-06
```
