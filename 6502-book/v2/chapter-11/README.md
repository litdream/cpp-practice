# Chapter 11: Audio Subsystem (The Apple II Speaker)

## 📖 Overview & Educational Philosophy
Welcome to Chapter 11! Now that we have a fully functional visual interface and interactive keyboard inputs, we are ready to bring our Apple II+ emulator to life with sound. In this chapter, we implement the hardware audio generation subsystem by emulating the physical Apple II speaker.

In physical hardware, the speaker is a simple paper cone connected to a flip-flop. Toggling this flip-flop changes the voltage applied to the speaker, pushing the cone out or pulling it in. By toggling this state rapidly, we generate sound waves. 

In this chapter, we:
1. **Intercept MMIO `$C030`**: Detect any read or write access to the speaker register.
2. **Track Exact Cycles**: Implement cycle-accurate timing updates to know *when* the toggle happened relative to the audio sample rate.
3. **Queue Audio Samples**: Synthesize a square wave by buffering speaker states and queueing them using **SDL2 Audio API** (`SDL_QueueAudio`).

## 🛠️ The Hardware Dance: Speaker Toggling
The Apple II speaker does not have a frequency register or volume control. The CPU must manually toggle the speaker by accessing `$C030`. If the programmer wants a 1 kHz tone, they must write code that accesses `$C030` exactly once every 500 microseconds. 

This technique, known as pulse-width modulation (PWM) or manual toggling, puts a heavy burden on the emulator to track cycles accurately. If we only checked the speaker state once per frame (16.6ms), we would miss almost all the audio transitions! By linking the CPU cycle counter with our audio sample generation, we achieve high-fidelity reproduction of classic retro game clicks, ticks, and multi-tone signals.

## ⚙️ Building and Running Chapter 11
The emulator will now boot into the Applesoft BASIC screen, and you should hear the classic boot beep!

### Build Steps:
```bash
mkdir build
cd build
cmake ..
make
./chapter-11
```
