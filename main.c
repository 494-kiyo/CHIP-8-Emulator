#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"

// Function prototypes
bool initializeSDL(SDL_Window** window, SDL_Renderer** renderer);
void closeSDL(SDL_Window* window, SDL_Renderer* renderer);
void drawGraphics(SDL_Renderer* renderer, bool display[]);

bool initializeSDL(SDL_Window** window, SDL_Renderer** renderer) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Create a window
    *window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCREEN_SCALE,
                               SCREEN_HEIGHT * SCREEN_SCALE, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Create a renderer
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void closeSDL(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void drawGraphics(SDL_Renderer* renderer, bool display[]) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            if (display[y * 64 + x]) {
                SDL_Rect rect = {x * SCREEN_SCALE, y * SCREEN_SCALE,
                                 SCREEN_SCALE, SCREEN_SCALE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

int main(int argc, char** argv) {
    srand(time(NULL));
    loadFontSet();
    loadFile();
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (!initializeSDL(&window, &renderer)) {
        return -1;
    }

    // Initialize display memory (64x32)
    //bool display[64 * 32] = {false};

    // Main loop
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        //SDL_PumpEvents();
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                // Handle key press or release events
                switch (e.key.keysym.sym) {
                    case SDLK_x: keyboard[0x0] = (e.type == SDL_KEYDOWN); break; // 0
                    case SDLK_1: keyboard[0x1] = (e.type == SDL_KEYDOWN); break; // 1
                    case SDLK_2: keyboard[0x2] = (e.type == SDL_KEYDOWN); break; // 2
                    case SDLK_3: keyboard[0x3] = (e.type == SDL_KEYDOWN); break; // 3
                    case SDLK_q: keyboard[0x4] = (e.type == SDL_KEYDOWN); break; // 4
                    case SDLK_w: keyboard[0x5] = (e.type == SDL_KEYDOWN); break; // 5
                    case SDLK_e: keyboard[0x6] = (e.type == SDL_KEYDOWN); break; // 6
                    case SDLK_a: keyboard[0x7] = (e.type == SDL_KEYDOWN); break; // 7
                    case SDLK_s: keyboard[0x8] = (e.type == SDL_KEYDOWN); break; // 8
                    case SDLK_d: keyboard[0x9] = (e.type == SDL_KEYDOWN); break; // 9
                    case SDLK_z: keyboard[0xA] = (e.type == SDL_KEYDOWN); break; // A
                    case SDLK_c: keyboard[0xB] = (e.type == SDL_KEYDOWN); break; // B
                    case SDLK_4: keyboard[0xC] = (e.type == SDL_KEYDOWN); break; // C
                    case SDLK_r: keyboard[0xD] = (e.type == SDL_KEYDOWN); break; // D
                    case SDLK_f: keyboard[0xE] = (e.type == SDL_KEYDOWN); break; // E
                    case SDLK_v: keyboard[0xF] = (e.type == SDL_KEYDOWN); break; // F
                    }
                }
        }
        emulateCycle();
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Draw CHIP-8 display
        if (drawFlag) {
            drawGraphics(renderer, screen);
        }

        // Render present
        SDL_RenderPresent(renderer);

        // Delay to cap frame rate
        SDL_Delay(16); // Cap frame rate at approximately 60 FPS

    }
    return 0;
}