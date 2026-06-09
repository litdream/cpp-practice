#include <iostream>
#include <iomanip>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Memory.h"
#include "SystemBus.h"
#include "CPU6502.h"
#include "Debugger.h"
#include "Video.h"
#include "Speaker.h"
#include "apple2plus_rom.h"

int main(int argc, char* argv[]) {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 10: Debugger & Inspection" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 1. Instantiate Core Topology
    Memory memory;
    Speaker speaker;
    SystemBus bus(&memory, &speaker);
    CPU6502 cpu(&bus);

    // 2. Load System ROM
    std::cout << "[Test 1] Loading System ROM for Kernel Traceability..." << std::endl;
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    // 3. Test Disassembler at $D000 (Start of ROM)
    std::cout << "[Test 2] Synthesizing Instruction Disassembly..." << std::endl;
    std::string assembly;
    uint16_t bytes = Debugger::disassemble(&bus, 0xD000, assembly);
    std::cout << "  Disassembled at $D000: " << assembly << " (Consumed: " << bytes << " bytes)" << std::endl;

    // 4. Test Memory Hex-Dumper on Zero-Page and Primary Display Map
    std::cout << "[Test 3] Triggering Localized System RAM Hex Dumps..." << std::endl;
    std::cout << "  Dumping Zero-Page Address space ($0000-$0020):" << std::endl;
    Debugger::dumpMemory(&bus, 0x0000, 0x20);

    // 5. Trigger Power-Up and dump CPU state
    std::cout << "[Test 4] Triggering Reset Vectors & Introspecting CPU Topology..." << std::endl;
    cpu.reset();
    Debugger::dumpState(cpu);

    // 6. Execute a few instructions and dump again
    // Handcraft code at $0300
    bus.write(0x0300, 0xA9);
    bus.write(0x0301, 0x42);
    bus.write(0x0302, 0x8D);
    bus.write(0x0303, 0x00);
    bus.write(0x0304, 0x02);

    cpu.PC = 0x0300; // Force PC to our handcrafted code
    std::cout << "Executing handcrafted code at $0300..." << std::endl;
    
    std::string dasm1, dasm2;
    Debugger::disassemble(&bus, 0x0300, dasm1);
    Debugger::disassemble(&bus, 0x0302, dasm2);
    std::cout << "  Trace: " << dasm1 << std::endl;
    cpu.execute(2); // Execute LDA
    std::cout << "  Trace: " << dasm2 << std::endl;
    cpu.execute(4); // Execute STA

    Debugger::dumpState(cpu);

    std::cout << "\n===========================================" << std::endl;
    std::cout << " Advanced Debugging Mechanisms SUCCESS." << std::endl;
    std::cout << " Launching Final Interactive System Emulator Window..." << std::endl;
    std::cout << "===========================================" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!speaker.init()) {
        std::cerr << "Warning: Speaker initialization failed. Running without sound." << std::endl;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    }

    Video video(&bus);
    cpu.reset(); // Full cold boot 

    const int SCALE = 3;
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 192;

    SDL_Window* window = SDL_CreateWindow(
        "Chapter 10: Advanced Debugging (ver6-vibe)", 
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
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

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
                    ch -= 32;
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
                    case SDLK_F1:
                        Debugger::dumpState(cpu);
                        break;
                    case SDLK_F2:
                        std::cout << "--- Zero Page Hex Dump ---" << std::endl;
                        Debugger::dumpMemory(&bus, 0x0000, 0x100);
                        break;
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

        cpu.execute(17000);
        speaker.flush(bus.getSystemCycles());
        video.update(backbuffer);

        SDL_UpdateTexture(texture, NULL, backbuffer->pixels, backbuffer->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    std::cout << "Shutting down gracefully." << std::endl;
    SDL_FreeSurface(backbuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
