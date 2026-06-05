#include "Video.h"
#include <SDL2/SDL_image.h>
#include <iostream>

// Include the XPM font
#include "/usr/local/google/home/raychung/prg/study-6502/ver5-vibe/res/charset40_IIplus.xpm"

const uint16_t Video::ROW_BASE[24] = {
    0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780,
    0x0428, 0x04A8, 0x0528, 0x05A8, 0x0628, 0x06A8, 0x0728, 0x07A8,
    0x0450, 0x04D0, 0x0550, 0x05D0, 0x0650, 0x06D0, 0x0750, 0x07D0
};

Video::Video(Bus* bus) : bus(bus), fontNormal(nullptr), fontInverse(nullptr) {
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

    SDL_Rect srcRect;
    srcRect.w = 8;
    srcRect.h = 8;

    SDL_Rect dstRect;
    dstRect.w = 8;
    dstRect.h = 8;

    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 40; ++col) {
            uint16_t addr = ROW_BASE[row] + col;
            uint8_t ch = bus->read(addr);

            // Apple II text styles:
            // Bit 7 and 6 determine style:
            // 00 -> Inverse
            // 01 -> Flashing
            // 10 -> Normal
            // 11 -> Normal
            uint8_t style = (ch >> 6) & 0x03;
            uint8_t idx = ch & 0x3F; // 64 shapes

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
