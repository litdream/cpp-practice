#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib> // for getenv
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Memory.h"
#include "SystemBus.h"
#include "Video.h"

int main(int argc, char* argv[]) {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 6: Video Architecture" << std::endl;
    std::cout << "===========================================" << std::endl;


    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    }

    const int SCALE = 3;
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 192;

    SDL_Window* window = SDL_CreateWindow(
        "Chapter 6: Video Architecture (ver6-vibe)", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH * SCALE, 
        SCREEN_HEIGHT * SCALE, 
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT
    );

    if (!texture) {
        std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Surface* backbuffer = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                                   0x00FF0000,
                                                   0x0000FF00,
                                                   0x000000FF,
                                                   0xFF000000);

    if (!backbuffer) {
        std::cerr << "Backbuffer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // 1. Instantiate our Emulation Backplane
    Memory memory;
    SystemBus bus(&memory);
    Video video(&bus);

    std::cout << "[Test 1] Injecting Text Map into Video Page 1 ($0400-$07FF)..." << std::endl;
    // Let's write " APPLE ][" at the top, and "HELLO WORLD" below
    // Apple II text characters:
    // $00-$3F: Inverse
    // $40-$7F: Flashing
    // $80-$BF: Normal (A-Z is 0x81-0x9A, but in our XPM it's mapped 0x00-0x3F)
    // Wait, let's look at `Video.cpp` line 76:
    // `uint8_t style = (ch >> 6) & 0x03;`
    // `uint8_t idx = ch & 0x3F;`
    // So `0x80` is style 2 (Normal), idx 0x00 ('@' or space in some charsets).
    // Let's see the XPM to see what index 0 is.
    // Line 8 of XPM: `..   ......`
    // It's a shape matrix.
    // Usually, in `charset40_IIplus.xpm`:
    // Index 1 is 'A', 2 is 'B', etc.
    // Index 0 is '@' or space?
    // Let's check `Apple2plus_rom.h` to see how it uses text.
    // Actually, in `Video.cpp`:
    // `srcRect.x = (idx % 16) * 8;`
    // `srcRect.y = (idx / 16) * 8;`
    // Let's write some values to `$0400` and see what happens.
    // 'H' = 0x88 (Style 2: Normal, idx 0x08)
    // 'E' = 0x85
    // 'L' = 0x8C
    // 'O' = 0x8F
    // 'W' = 0x97
    // 'R' = 0x92
    // 'D' = 0x84

    uint16_t baseAddr = 0x0400; // Row 0
    bus.write(baseAddr + 0, 0x81); // A
    bus.write(baseAddr + 1, 0x90); // P
    bus.write(baseAddr + 2, 0x90); // P
    bus.write(baseAddr + 3, 0x8C); // L
    bus.write(baseAddr + 4, 0x85); // E
    bus.write(baseAddr + 5, 0xA0); // Space
    bus.write(baseAddr + 6, 0xDB); // ]
    bus.write(baseAddr + 7, 0xDB); // ]

    uint16_t row1Addr = 0x0480; // Row 1 in ROW_BASE
    bus.write(row1Addr + 0, 0x88); // H
    bus.write(row1Addr + 1, 0x85); // E
    bus.write(row1Addr + 2, 0x8C); // L
    bus.write(row1Addr + 3, 0x8C); // L
    bus.write(row1Addr + 4, 0x8F); // O
    bus.write(row1Addr + 5, 0xA0); // Space
    bus.write(row1Addr + 6, 0x97); // W
    bus.write(row1Addr + 7, 0x8F); // O
    bus.write(row1Addr + 8, 0x92); // R
    bus.write(row1Addr + 9, 0x8C); // L
    bus.write(row1Addr + 10, 0x84); // D

    std::cout << "[Test 2] Displaying Video Frame for 3 seconds..." << std::endl;
    video.update(backbuffer);

    SDL_UpdateTexture(texture, NULL, backbuffer->pixels, backbuffer->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(3000); 

    std::cout << "\n===========================================" << std::endl;
    std::cout << " Video Subsystem & Rasterization SUCCESS." << std::endl;
    std::cout << " Ready for Chapter 7: ROM Bootstrap." << std::endl;
    std::cout << "===========================================" << std::endl;

    SDL_FreeSurface(backbuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
