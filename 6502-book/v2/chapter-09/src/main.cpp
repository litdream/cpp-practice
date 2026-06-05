#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Memory.h"
#include "SystemBus.h"
#include "CPU6502.h"
#include "Video.h"
#include "apple2plus_rom.h"

int runInteractive(Memory& memory, SystemBus& bus, CPU6502& cpu, Video& video) {
    const int SCALE = 3;
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 192;

    SDL_Window* window = SDL_CreateWindow(
        "Chapter 9: The Interactive Loop (ver6-vibe)", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH * SCALE, 
        SCREEN_HEIGHT * SCALE, 
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
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
        SDL_FreeSurface(backbuffer);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
    }

    SDL_StartTextInput();

    bool quit = false;
    SDL_Event e;

    std::cout << "Master Emulation Loop Started. Accepting keystrokes..." << std::endl;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_TEXTINPUT) {
                char ch = e.text.text[0];
                if (ch >= 'a' && ch <= 'z') {
                    ch -= 32; // Uppercase for Apple II+
                }
                bus.setKey((uint8_t)ch);
            }
            else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                uint8_t appleKey = 0;

                switch (sym) {
                    case SDLK_RETURN:    appleKey = 0x0D; break;
                    case SDLK_BACKSPACE: appleKey = 0x08; break;
                    case SDLK_LEFT:      appleKey = 0x08; break;
                    case SDLK_RIGHT:     appleKey = 0x15; break;
                    case SDLK_ESCAPE:    quit = true; break;
                    default:
                        if (sym >= SDLK_a && sym <= SDLK_z) {
                            if (e.key.keysym.mod & KMOD_CTRL) {
                                appleKey = sym & 0x1F;
                            } else {
                                appleKey = sym - 'a' + 'A';
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

        // Apple II runs at ~1MHz. 60Hz frame is ~16,666 cycles.
        cpu.execute(17000);

        video.update(backbuffer);

        SDL_UpdateTexture(texture, NULL, backbuffer->pixels, backbuffer->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(backbuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}

int runHeadless(Memory& memory, SystemBus& bus, CPU6502& cpu) {
    std::cout << "[Headless Verification Mode] Simulating Keyboard Interrupt and Strobe Clears..." << std::endl;

    cpu.reset();
    std::cout << "[Test 1] Booting Kernel to stabilize at Input Prompt..." << std::endl;
    cpu.execute(500000);

    std::cout << "[Test 2] Synthesizing Hardware ASCII 'J' into mapped $C000..." << std::endl;
    bus.setKey('J');

    uint8_t strobeRead = bus.read(0xC000);
    std::cout << "  Read $C000 (Strobe Bit 7 High): 0x" << std::hex << (int)strobeRead << std::dec << std::endl;

    std::cout << "[Test 3] Processing CPU cycles to allow Firmware Strobe Clear..." << std::endl;
    // Execute sufficient cycles for the polling loop to catch and clear the strobe
    cpu.execute(50000); 

    uint8_t afterStrobeRead = bus.read(0xC000);
    std::cout << "  Read $C000 (Strobe Bit 7 Low): 0x" << std::hex << (int)afterStrobeRead << std::dec << std::endl;

    // Check if the Strobe Bit 7 was successfully cleared (Bit 7 == 0)
    if ((strobeRead & 0x80) == 0x80 && (afterStrobeRead & 0x80) == 0x00) {
        std::cout << "\n===========================================" << std::endl;
        std::cout << " Interactive Loop Integration SUCCESS." << std::endl;
        std::cout << " Ready for Chapter 10: Advanced Debugging." << std::endl;
        std::cout << "===========================================" << std::endl;
        return 0;
    } else {
        std::cout << "\n[ERROR] Keyboard Strobe Handshake Verification FAILED!" << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 9: The Interactive Loop" << std::endl;
    std::cout << "===========================================" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    }

    Memory memory;
    SystemBus bus(&memory);
    CPU6502 cpu(&bus);
    Video video(&bus);

    std::cout << "Loading System ROM..." << std::endl;
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    std::cout << "Triggering Electrical Reset Sequence..." << std::endl;
    cpu.reset();

    std::cout << "Starting Interactive Loop. OS Window should appear now." << std::endl;
    int result = runInteractive(memory, bus, cpu, video);

    IMG_Quit();
    SDL_Quit();

    return result;
}
