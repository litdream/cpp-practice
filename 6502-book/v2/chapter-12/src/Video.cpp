#include "Video.h"
#include "SystemBus.h"
#include <SDL2/SDL_image.h>
#include <iostream>

// Include the XPM font
#include "../res/charset40_IIplus.xpm"

struct RGB { uint8_t r, g, b; };
const RGB GR_COLORS[16] = {
    {0, 0, 0},         // 0: Black
    {224, 16, 96},     // 1: Magenta
    {32, 32, 192},     // 2: Dark Blue
    {224, 32, 224},    // 3: Purple
    {0, 128, 0},       // 4: Dark Green
    {128, 128, 128},   // 5: Grey 1
    {0, 0, 255},       // 6: Medium Blue
    {64, 192, 255},    // 7: Light Blue
    {128, 64, 0},      // 8: Brown
    {255, 128, 0},     // 9: Orange
    {128, 128, 128},   // 10: Grey 2
    {255, 192, 192},   // 11: Pink
    {0, 255, 0},       // 12: Green
    {255, 255, 0},     // 13: Yellow
    {0, 255, 255},     // 14: Aqua
    {255, 255, 255}    // 15: White
};

const uint16_t Video::ROW_BASE[24] = {
    0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780,
    0x0428, 0x04A8, 0x0528, 0x05A8, 0x0628, 0x06A8, 0x0728, 0x07A8,
    0x0450, 0x04D0, 0x0550, 0x05D0, 0x0650, 0x06D0, 0x0750, 0x07D0
};

Video::Video(SystemBus* bus) : bus(bus), fontNormal(nullptr), fontInverse(nullptr) {
    fontInverse = IMG_ReadXPMFromArray(charset40_IIplus_xpm);
    fontNormal = IMG_ReadXPMFromArray(charset40_IIplus_xpm);

    if (!fontInverse || !fontNormal) {
        std::cerr << "Failed to load font surfaces: " << IMG_GetError() << std::endl;
    } else {
        // Invert colors on fontNormal only
        if (fontNormal->format->palette && fontNormal->format->palette->ncolors >= 2) {
            SDL_Palette* palette = fontNormal->format->palette;
            SDL_Color colors[2];
            colors[0] = palette->colors[1];
            colors[1] = palette->colors[0];
            SDL_SetPaletteColors(palette, colors, 0, 2);
        } else if (fontNormal->format->BytesPerPixel == 4) {
            SDL_LockSurface(fontNormal);
            Uint32* pixels = (Uint32*)fontNormal->pixels;
            int count = fontNormal->w * fontNormal->h;
            for (int i = 0; i < count; ++i) {
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixels[i], fontNormal->format, &r, &g, &b, &a);
                pixels[i] = SDL_MapRGBA(fontNormal->format, 255 - r, 255 - g, 255 - b, a);
            }
            SDL_UnlockSurface(fontNormal);
        }
    }
}

Video::~Video() {
    if (fontNormal) SDL_FreeSurface(fontNormal);
    if (fontInverse) SDL_FreeSurface(fontInverse);
}

void Video::update(SDL_Surface* surface) {
    if (!fontNormal || !fontInverse) return;

    // Toggle blink state every 20 frames (~3Hz)
    frameCount++;
    if (frameCount >= 20) {
        frameCount = 0;
        blinkState = !blinkState;
    }

    bool isGraphics = bus->isGraphicsMode();
    bool isMixed = bus->isMixedMode();
    bool isHires = bus->isHiresMode();

    if (!isGraphics) {
        renderText(surface, 0, 24);
    } else {
        if (!isHires) {
            if (isMixed) {
                renderLores(surface, 0, 20);
                renderText(surface, 20, 24);
            } else {
                renderLores(surface, 0, 24);
            }
        } else {
            // Hires Mode placeholder: fill with dark blue
            SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 0, 0, 128, 255));
        }
    }
}

void Video::renderText(SDL_Surface* surface, int startRow, int endRow) {
    SDL_Rect srcRect;
    srcRect.w = 8;
    srcRect.h = 8;

    SDL_Rect dstRect;
    dstRect.w = 8;
    dstRect.h = 8;

    for (int row = startRow; row < endRow; ++row) {
        for (int col = 0; col < 40; ++col) {
            uint16_t addr = ROW_BASE[row] + col;
            uint8_t ch = bus->read(addr);

            uint8_t style = (ch >> 6) & 0x03;
            uint8_t idx = ch & 0x3F;

            SDL_Surface* srcSurf = fontNormal;

            if (style == 0) {
                srcSurf = fontInverse;
            } else if (style == 1) {
                srcSurf = blinkState ? fontInverse : fontNormal;
            }

            srcRect.x = (idx % 16) * 8;
            srcRect.y = (idx / 16) * 8;

            dstRect.x = col * 8;
            dstRect.y = row * 8;

            SDL_BlitSurface(srcSurf, &srcRect, surface, &dstRect);
        }
    }
}

void Video::renderLores(SDL_Surface* surface, int startRow, int endRow) {
    SDL_Rect rect;
    rect.w = 8;
    rect.h = 4;

    for (int row = startRow; row < endRow; ++row) {
        for (int col = 0; col < 40; ++col) {
            uint16_t addr = ROW_BASE[row] + col;
            uint8_t val = bus->read(addr);

            uint8_t topColorIdx = val & 0x0F;
            uint8_t bottomColorIdx = (val >> 4) & 0x0F;

            // Top block
            rect.x = col * 8;
            rect.y = row * 8;
            RGB topColor = GR_COLORS[topColorIdx];
            uint32_t topColorMap = SDL_MapRGBA(surface->format, topColor.r, topColor.g, topColor.b, 255);
            SDL_FillRect(surface, &rect, topColorMap);

            // Bottom block
            rect.y = row * 8 + 4;
            RGB bottomColor = GR_COLORS[bottomColorIdx];
            uint32_t bottomColorMap = SDL_MapRGBA(surface->format, bottomColor.r, bottomColor.g, bottomColor.b, 255);
            SDL_FillRect(surface, &rect, bottomColorMap);
        }
    }
}
