#pragma once
#include <SDL2/SDL.h>
#include <vector>

class Speaker {
public:
    Speaker();
    ~Speaker();

    bool init();
    void toggle(uint64_t cycle);
    void flush(uint64_t currentCycle);
    void close();

private:
    SDL_AudioDeviceID audioDevice = 0;
    uint64_t sampleCount = 0;
    int16_t speakerState = -3000; // Low state, reduced amplitude to avoid clipping/blasting
    std::vector<int16_t> audioBuffer;

    const uint32_t SAMPLE_RATE = 44100;
    const uint32_t CPU_FREQ = 1023000;

    void playBuffer();
};
