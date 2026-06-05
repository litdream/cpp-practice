#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "Bus.h"

class Video {
public:
    Video(Bus* bus);
    ~Video();

    void update(SDL_Surface* surface);

private:
    Bus* bus;
    SDL_Surface* fontNormal;
    SDL_Surface* fontInverse;
    uint32_t frameCount = 0;
    bool blinkState = false;

    // Apple II text row base addresses
    static const uint16_t ROW_BASE[24];
};
