#include "Speaker.h"
#include <iostream>

Speaker::Speaker() {
    audioBuffer.reserve(4096);
}

Speaker::~Speaker() {
    close();
}

bool Speaker::init() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL Audio Init Failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = nullptr; // Use queue

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audioDevice == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_PauseAudioDevice(audioDevice, 0); // Start playing silence
    return true;
}

void Speaker::toggle(uint64_t cycle) {
    if (audioDevice == 0) return;
    // Generate samples up to this cycle
    uint64_t nextSampleCycle = (sampleCount * CPU_FREQ) / SAMPLE_RATE;
    while (nextSampleCycle <= cycle) {
        audioBuffer.push_back(speakerState);
        sampleCount++;
        nextSampleCycle = (sampleCount * CPU_FREQ) / SAMPLE_RATE;
    }
    // Toggle state
    speakerState = -speakerState;
}

void Speaker::flush(uint64_t currentCycle) {
    if (audioDevice == 0) return;
    // Generate samples up to the end of the frame
    uint64_t nextSampleCycle = (sampleCount * CPU_FREQ) / SAMPLE_RATE;
    while (nextSampleCycle <= currentCycle) {
        audioBuffer.push_back(speakerState);
        sampleCount++;
        nextSampleCycle = (sampleCount * CPU_FREQ) / SAMPLE_RATE;
    }
    playBuffer();
}

void Speaker::playBuffer() {
    if (audioDevice != 0 && !audioBuffer.empty()) {
        SDL_QueueAudio(audioDevice, audioBuffer.data(), audioBuffer.size() * sizeof(int16_t));
        audioBuffer.clear();
    }
}

void Speaker::close() {
    if (audioDevice != 0) {
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }
}
