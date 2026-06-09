#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
class SystemBus;

class Video {
public:
    Video(SystemBus* bus);
    ~Video();

    void update(SDL_Surface* surface);

private:
    void renderText(SDL_Surface* surface, int startRow, int endRow);
    void renderLores(SDL_Surface* surface, int startRow, int endRow);

    SystemBus* bus;
    SDL_Surface* fontNormal;
    SDL_Surface* fontInverse;
    uint32_t frameCount = 0;
    bool blinkState = false;

    // Apple II text row base addresses
    static const uint16_t ROW_BASE[24];
};
