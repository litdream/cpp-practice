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
#include "DiskDrive.h"
#include "apple2plus_rom.h"

int main(int argc, char* argv[]) {
    std::cout << "==========================================================" << std::endl;
    std::cout << " 6502-book Chapter 16: Floppy Disk & HLE Booting" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Instantiate Core Topology
    Memory memory;
    Speaker speaker;
    DiskDrive diskDrive;
    SystemBus bus(&memory, &speaker, &diskDrive);
    CPU6502 cpu(&bus);

    // 2. Load System ROM
    std::cout << "Loading System ROM..." << std::endl;
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    // 3. Load Floppy Disk Image
    std::string diskPath = "SNAKEBYTE.DSK";
    if (argc > 1) {
        diskPath = argv[1];
    }
    std::cout << "Loading Floppy Disk: " << diskPath << "..." << std::endl;
    if (!diskDrive.loadDisk(diskPath)) {
        std::cerr << "Warning: Could not load disk image: " << diskPath << "!" << std::endl;
    }

    // 4. Set up Slot 6 ROM Boot Signatures in virtual Memory
    // These signatures are read by the Apple II Autostart ROM reset scan routine
    // to detect that a bootable disk controller is present in Slot 6.
    memory.writeSlotROM(0xC601, 0x20); // LDX #$20 signature
    memory.writeSlotROM(0xC603, 0x00); // LDY #$00 signature
    memory.writeSlotROM(0xC605, 0x03); // LDX #$03 signature
    memory.writeSlotROM(0xC607, 0x3C); // Autostart ROM signature
    memory.writeSlotROM(0xC63C, 0x3C); // ProDOS slot-ROM signature

    // 5. Initialize Video Soft Switches to default (Text mode active)
    bus.write(0xC051, 0); // Text Mode On
    bus.write(0xC054, 0); // Page 1 Active

    // 6. Setup SDL2
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
    cpu.reset(); // Cold boot CPU (ROM will scan slots and boot from Slot 6 automatically!)

    const int SCALE = 3;
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 192;

    SDL_Window* window = SDL_CreateWindow(
        "Chapter 16: Floppy Disk & HLE Booting", 
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

    std::cout << "\nEmulator running. Booting disk..." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - [F1] - Dump CPU State to terminal" << std::endl;
    std::cout << "  - [F2] - Dump Zero-Page memory to terminal" << std::endl;
    std::cout << "  - [F3] - Force Manual Boot from Slot 6 (Re-execute $C600)" << std::endl;
    std::cout << "  - [ESC] - Quit emulator" << std::endl;

    bool quit = false;
    SDL_Event e;

    // Emulate 1.023 MHz (approx 17000 cycles per 60Hz frame)
    const int CYCLES_PER_FRAME = 17050; 

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                std::cout << "[Emulator Debug] Received SDL_QUIT event." << std::endl;
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
                    case SDLK_ESCAPE:
                        std::cout << "[Emulator Debug] Escape key pressed. Exiting..." << std::endl;
                        quit = true;
                        break;
                    case SDLK_F1:
                        Debugger::dumpState(cpu);
                        break;
                    case SDLK_F2:
                        std::cout << "--- Zero Page Hex Dump ---" << std::endl;
                        Debugger::dumpMemory(&bus, 0x0000, 0x100);
                        break;
                    case SDLK_F3:
                        std::cout << "[Emulator Switch] Forcing Slot 6 Boot Sequence..." << std::endl;
                        cpu.reset(); // Clear CPU state
                        cpu.PC = 0xC600; // Point to Slot 6 entry
                        break;
                    default:
                        if (sym >= SDLK_a && sym <= SDLK_z) {
                            if (e.key.keysym.mod & KMOD_CTRL) {
                                appleKey = sym & 0x1F;
                            }
                        }
                        break;
                }

                if (appleKey > 0) {
                    bus.setKey(appleKey);
                }
            }
        }

        cpu.execute(CYCLES_PER_FRAME); 
        speaker.flush(bus.getSystemCycles());
        video.update(backbuffer);

        SDL_UpdateTexture(texture, NULL, backbuffer->pixels, backbuffer->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    std::cout << "Shutting down emulator." << std::endl;
    SDL_FreeSurface(backbuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
