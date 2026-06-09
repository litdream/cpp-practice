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

// Helper to write text to a specific Apple II Text Page
void drawTextPattern(Memory& memory, uint16_t baseAddr, const std::string& text, int row) {
    const uint16_t ROW_BASE[24] = {
        0x0000, 0x0080, 0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380,
        0x0028, 0x00A8, 0x0128, 0x01A8, 0x0228, 0x02A8, 0x0328, 0x03A8,
        0x0050, 0x00D0, 0x0150, 0x01D0, 0x0250, 0x02D0, 0x0350, 0x03D0
    };
    
    if (row < 0 || row >= 24) return;
    uint16_t rowStart = baseAddr + ROW_BASE[row];
    for (size_t i = 0; i < text.length() && i < 40; ++i) {
        // Apple II character set: Bit 7 set to 1 for normal ASCII
        uint8_t ch = text[i] | 0x80;
        memory.write(rowStart + i, ch);
    }
}

// Helper to draw a test pattern on an Apple II HGR Page
void drawHgrPattern(Memory& memory, uint16_t baseAddr, int patternType) {
    // Clear page first (8KB)
    for (uint16_t offset = 0; offset < 0x2000; ++offset) {
        memory.write(baseAddr + offset, 0x00);
    }

    for (int y = 0; y < 192; ++y) {
        // Interleaved addressing calculation
        int block = y / 64;
        int row_in_block = y % 64;
        int group = row_in_block / 8;
        int line_in_group = row_in_block % 8;
        uint16_t rowAddr = baseAddr + (line_in_group * 0x0400) + (group * 0x0080) + (block * 0x0028);

        for (int col = 0; col < 40; ++col) {
            uint8_t byteVal = 0;
            if (patternType == 1) {
                // Pattern 1: Vertical stripes (alternating bits)
                // 0x2A = 00101010, 0x55 = 01010101 (Green / Violet stripes)
                byteVal = (col % 2 == 0) ? 0x2A : 0x55;
            } else if (patternType == 2) {
                // Pattern 2: Grid + Blue/Orange background
                if (y % 16 == 0 || col % 5 == 0) {
                    byteVal = 0x7F; // White grid lines
                } else {
                    // 0xD5 = 11010101, 0xAA = 10101010 (Blue / Orange stripes, MSB set)
                    byteVal = (col % 2 == 0) ? 0xD5 : 0xAA;
                }
            }
            memory.write(rowAddr + col, byteVal);
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "==========================================================" << std::endl;
    std::cout << " 6502-book Chapter 14: HGR2 and Page 2 Soft Switches" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Instantiate Core Topology
    Memory memory;
    Speaker speaker;
    SystemBus bus(&memory, &speaker);
    CPU6502 cpu(&bus);

    // 2. Load System ROM
    std::cout << "Loading System ROM..." << std::endl;
    memory.loadROM((const uint8_t*)Apple2plus_rom, sizeof(Apple2plus_rom) - 1);

    // 3. Pre-populate Video Memory with Test Patterns
    std::cout << "Pre-populating Text Page 1 ($0400) and Page 2 ($0800)..." << std::endl;
    // Clear pages first
    for (uint16_t addr = 0x0400; addr < 0x0C00; ++addr) {
        memory.write(addr, 0xA0); // Apple II space character (0xA0 is space with bit 7 set)
    }

    // Write text to Page 1
    drawTextPattern(memory, 0x0400, "      *** APPLE II PAGE 1 ACTIVE ***", 1);
    drawTextPattern(memory, 0x0400, "GRAPHICS MODE IS ON, MIXED SCREEN", 20);
    drawTextPattern(memory, 0x0400, "THIS IS THE TEXT PORTION OF PAGE 1", 21);
    drawTextPattern(memory, 0x0400, "HGR1 ADDR: $2000-$3FFF", 22);
    drawTextPattern(memory, 0x0400, "PRESS [2] FOR PAGE 2, [G]/[T] TEXT/GRAPH", 23);

    // Write text to Page 2
    drawTextPattern(memory, 0x0800, "      *** APPLE II PAGE 2 ACTIVE ***", 1);
    drawTextPattern(memory, 0x0800, "GRAPHICS MODE IS ON, MIXED SCREEN", 20);
    drawTextPattern(memory, 0x0800, "THIS IS THE TEXT PORTION OF PAGE 2", 21);
    drawTextPattern(memory, 0x0800, "HGR2 ADDR: $4000-$5FFF", 22);
    drawTextPattern(memory, 0x0800, "PRESS [1] FOR PAGE 1, [G]/[T] TEXT/GRAPH", 23);

    std::cout << "Pre-populating HGR Page 1 ($2000) and Page 2 ($4000)..." << std::endl;
    drawHgrPattern(memory, 0x2000, 1); // Vertical stripes on Page 1
    drawHgrPattern(memory, 0x4000, 2); // Grid pattern on Page 2

    // 4. Initialize Video Soft Switches to Hires, Mixed, Graphics, Page 1
    bus.write(0xC050, 0); // Graphics Mode On
    bus.write(0xC053, 0); // Mixed Mode On
    bus.write(0xC057, 0); // Hires Mode On
    bus.write(0xC054, 0); // Page 1 Active

    // 5. Setup SDL2
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
    cpu.reset(); // Cold boot CPU

    const int SCALE = 3;
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 192;

    SDL_Window* window = SDL_CreateWindow(
        "Chapter 14: HGR2 & Page 2 Soft Switches", 
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

    std::cout << "\nInteractive Controls Available:" << std::endl;
    std::cout << "  [1] - Select Page 1 (Clear/Select HGR1 & Text Page 1)" << std::endl;
    std::cout << "  [2] - Select Page 2 (Select HGR2 & Text Page 2)" << std::endl;
    std::cout << "  [G] - Set to Graphics Mode" << std::endl;
    std::cout << "  [T] - Set to Text Mode" << std::endl;
    std::cout << "  [M] - Set to Mixed Mode (Graphics + 4 Lines Text)" << std::endl;
    std::cout << "  [F] - Set to Full Screen (No Text)" << std::endl;
    std::cout << "  [H] - Set to Hires Graphics" << std::endl;
    std::cout << "  [L] - Set to Lores Graphics" << std::endl;
    std::cout << "  [ESC] - Quit" << std::endl;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                switch (sym) {
                    case SDLK_ESCAPE: quit = true; break;
                    case SDLK_1:
                        std::cout << "Soft Switch Triggered: Select Page 1 ($C054)" << std::endl;
                        bus.write(0xC054, 0);
                        break;
                    case SDLK_2:
                        std::cout << "Soft Switch Triggered: Select Page 2 ($C055)" << std::endl;
                        bus.write(0xC055, 0);
                        break;
                    case SDLK_g:
                        std::cout << "Soft Switch Triggered: Set Graphics Mode ($C050)" << std::endl;
                        bus.write(0xC050, 0);
                        break;
                    case SDLK_t:
                        std::cout << "Soft Switch Triggered: Set Text Mode ($C051)" << std::endl;
                        bus.write(0xC051, 0);
                        break;
                    case SDLK_f:
                        std::cout << "Soft Switch Triggered: Set Full Screen ($C052)" << std::endl;
                        bus.write(0xC052, 0);
                        break;
                    case SDLK_m:
                        std::cout << "Soft Switch Triggered: Set Mixed Mode ($C053)" << std::endl;
                        bus.write(0xC053, 0);
                        break;
                    case SDLK_l:
                        std::cout << "Soft Switch Triggered: Set Lores Mode ($C056)" << std::endl;
                        bus.write(0xC056, 0);
                        break;
                    case SDLK_h:
                        std::cout << "Soft Switch Triggered: Set Hires Mode ($C057)" << std::endl;
                        bus.write(0xC057, 0);
                        break;
                    default:
                        // Forward other key events to Apple II keyboard buffer
                        char ch = 0;
                        if (sym >= SDLK_a && sym <= SDLK_z) ch = sym - 'a' + 'A';
                        else if (sym >= SDLK_0 && sym <= SDLK_9) ch = sym;
                        else if (sym == SDLK_SPACE) ch = ' ';
                        else if (sym == SDLK_RETURN) ch = 0x0D;
                        
                        if (ch > 0) {
                            bus.setKey(ch);
                        }
                        break;
                }
            }
        }

        // Run CPU to keep any background process active
        cpu.execute(1000); 
        speaker.flush(bus.getSystemCycles());
        video.update(backbuffer);

        SDL_UpdateTexture(texture, NULL, backbuffer->pixels, backbuffer->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    std::cout << "Graceful shutdown." << std::endl;
    SDL_FreeSurface(backbuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
