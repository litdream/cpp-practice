#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(int argc, char* argv[]) {
    std::cout << "===========================================" << std::endl;
    std::cout << " 6502-book Chapter 2: The Skeleton Frame" << std::endl;
    std::cout << " Initializing SDL2 Subsystems..." << std::endl;
    std::cout << "===========================================" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    }

    const int SCALE = 3;
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 192;

    SDL_Window* window = SDL_CreateWindow(
        "Chapter 2: The Skeleton Frame (ver6-vibe)", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH * SCALE, 
        SCREEN_HEIGHT * SCALE, 
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, 
        -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create a backbuffer surface (320x192) as a staging pixel buffer
    SDL_Surface* backbuffer = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                                   0x00FF0000,
                                                   0x0000FF00,
                                                   0x000000FF,
                                                   0xFF000000);

    if (!backbuffer) {
        std::cerr << "Backbuffer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create a streaming texture for streaming the backbuffer pixels to the GPU
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
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    std::cout << "Master Emulation Loop Started. Listening for events..." << std::endl;
    std::cout << "Press 'Escape' or close the window to exit." << std::endl;

    while (!quit) {
        // 1. Event Polling (Heartbeat & Input Capture)
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
            }
        }

        // 2. Clear the backbuffer to a deep, vintage CRT dark green
        // #001A00 (R: 0, G: 26, B: 0)
        SDL_FillRect(backbuffer, NULL, SDL_MapRGB(backbuffer->format, 0, 26, 0));

        // In future chapters, this is exactly where the CPU will execute cycles
        // and the Video subsystem will draw pixels into our backbuffer.

        // 3. Upload our staging backbuffer pixels to the GPU streaming texture
        SDL_UpdateTexture(texture, NULL, backbuffer->pixels, backbuffer->pitch);

        // 4. Render to OS Window
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    std::cout << "Shutting down Skeleton Frame gracefully..." << std::endl;

    // Clean up all initialized resources
    SDL_FreeSurface(backbuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
