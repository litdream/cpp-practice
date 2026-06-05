# Chapter 9: The Interactive Loop & Event Integration

## 📖 Overview & Educational Philosophy
Welcome to Chapter 9! You have awakened the System Kernel by executing the electrical Reset Vector. Now, it is time for the final structural interconnect: the **Human Interface Vector**.

In this chapter, we integrate the modern host operating system hardware keyboard events into our virtual motherboard space. We translate modern SDL keystrokes into Apple II high-bit ASCII hardware strobes, mutating the mapped $C000 space while leveraging the $C010 strobe clear hooks to synchronize the master emulation loop.

## 🛠️ The Data Loop
The emulation cycle transforms from a linear test script into a continuous 60Hz heartbeat synchronization matrix:
1. **Event Capture**: SDL polling catches host OS keystrokes.
2. **Matrix Strobe**: Modern ASCII maps to $C000 with Bit 7 set high.
3. **Execution Synchronization**: CPU advances at ~1.023 MHz, and $C010 reads flush the strobe.
4. **Display Refresh**: High-fidelity video rasterization updates to the window.

## ⚙️ Building and Running Chapter 9
Our `main.cpp` executes the interactive GUI runtime environment! If run in a headless environment, it executes an automated strobe latency and intercept verification test suite.

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-09
```
