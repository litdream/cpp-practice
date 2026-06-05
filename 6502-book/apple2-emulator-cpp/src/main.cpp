#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // For Video font loading

#include "Memory.h"
#include "SystemBus.h"
#include "CPU6502.h"
#include "Video.h"
#include "apple2plus_rom.h"

int main(int argc, char* argv[]) {
    std::cout << "Initializing ver6-vibe Apple II+ Emulator..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG; // Font might be XPM, but good to init
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        // XPM might work without init, but good to check
    }

#define SCALE 3

    SDL_Window* window = SDL_CreateWindow("ver6-vibe Apple II+", 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          320 * SCALE, 192 * SCALE, 
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create hardware renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create backbuffer surface (320x192) as a staging buffer
    SDL_Surface* backbuffer = SDL_CreateRGBSurface(0, 320, 192, 32,
                                                   0x00FF0000,
                                                   0x0000FF00,
                                                   0x000000FF,
                                                   0xFF000000);

    if (!backbuffer) {
        std::cerr << "Backbuffer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create streaming texture for streaming pixels to GPU
    SDL_Texture* texture = SDL_CreateTexture(renderer, 
                                             SDL_PIXELFORMAT_ARGB8888, 
                                             SDL_TEXTUREACCESS_STREAMING, 
                                             320, 192);
    if (!texture) {
        std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Instantiate Emulator components
    Memory memory;
    SystemBus bus(&memory);
    CPU6502 cpu(&bus);
    Video video(&bus);

    // Load ROM
    std::cout << "Loading Apple II+ ROM..." << std::endl;
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    // Reset CPU
    cpu.reset();

    SDL_StartTextInput();

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_TEXTINPUT) {
                char ch = e.text.text[0];
                if (ch >= 'a' && ch <= 'z') {
                    ch -= 32; // Convert to uppercase for Apple II+
                }
                bus.setKey((uint8_t)ch);
            }
            else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                uint8_t appleKey = 0;

                switch (sym) {
                    case SDLK_RETURN:   
                        appleKey = 0x0D; 
#if TRACE
                        cpu.startTrace(50000); // Trace 50k instructions on Enter
#endif
                        break;
                    case SDLK_BACKSPACE: appleKey = 0x08; break;
                    case SDLK_LEFT:      appleKey = 0x08; break;
                    case SDLK_RIGHT:     appleKey = 0x15; break;
                    case SDLK_ESCAPE:    appleKey = 0x1B; break;
                    default:
                        // Fallback for letters and numbers on Wayland
                        if (sym >= SDLK_a && sym <= SDLK_z) {
                            if (e.key.keysym.mod & KMOD_CTRL) {
                                appleKey = sym & 0x1F; // Map to control character
                            } else {
                                appleKey = sym - 'a' + 'A'; // Convert to uppercase
                            }
                        } else if (sym >= SDLK_0 && sym <= SDLK_9) {
                            appleKey = sym;
                        } else if (sym == SDLK_SPACE) {
                            appleKey = ' ';
                        }
                        break;
                }

                if (appleKey > 0) {
                    bus.setKey(appleKey);
                }
            }
        }

        // Emulation loop
        // Apple II runs at ~1MHz. 60Hz frame is ~16666 cycles.
        cpu.execute(17000);

        // Render to backbuffer
        video.update(backbuffer);

        // Upload backbuffer to GPU texture
        SDL_UpdateTexture(texture, NULL, backbuffer->pixels, backbuffer->pitch);

        // Clear, copy texture, and present
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(backbuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
