# Chapter 2: The Skeleton Frame – Initializing SDL2

## 📖 Overview & Educational Philosophy
Welcome to Chapter 2! Now that our development environment is verified and our foundational bootstrap headers are in place, it’s time to construct the physical "casing" and monitor for our emulated machine. 

In systems programming and emulation, before we can fetch a single instruction or write to memory, we need a robust **execution loop** and a mechanism to render our machine’s internal state to the host operating system. We call this foundational structure **The Skeleton Frame**.

## 🛠️ Technical Focus: SDL2 Subsystems
In this chapter, we will interface with **SDL2 (Simple DirectMedia Layer)** to:
1. Initialize the primary video and event subsystems (`SDL_INIT_VIDEO`).
2. Spawn an OS-level window mapped to the historical Apple II screen resolution.
3. Attach a hardware-accelerated 2D renderer to handle our pixel buffers.
4. Establish the **Master Emulation Loop**—a continuous cycle that polls for host OS input events and ticks our virtual hardware forward.

## 📐 Resolution & Scaling
The original Apple II display generated an NTSC signal with a resolution of **280×192** in High-Resolution graphics mode, and **40×24** characters in Text mode. To map this nicely onto modern High-DPI monitors, we scale our emulated framebuffer (320×192, allowing for a standard border region) by a factor of **3x** (resulting in a 960×576 host window).

## ⚙️ Building and Running Chapter 2
You will find the source in `src/main.cpp`. By compiling and running this chapter, you should see an empty, dark green emulated monitor window appear on your screen. You can close it either by pressing the **Escape** key or by closing the window directly.

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-02
```
