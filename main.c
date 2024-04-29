#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096
#define MEMORY_START 0x200

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SCALE 8

#define OPCODE_X(opcode) ((opcode) & 0xF)

unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

uint16_t keyboard[16];
uint8_t reg[16]; // 16 general purpose registers
uint8_t SP; // stack pointer
uint16_t I; // current address
uint16_t PC = 0x200; // next address
uint16_t stack[16]; // stack
uint16_t memory[MEMORY_SIZE];
uint8_t delay;
uint8_t sound;
bool screen [SCREEN_WIDTH * SCREEN_HEIGHT] = {false};
bool drawFlag = false;

// Function prototypes
bool initializeSDL(SDL_Window** window, SDL_Renderer** renderer);
void closeSDL(SDL_Window* window, SDL_Renderer* renderer);
void drawGraphics(SDL_Renderer* renderer, bool display[]);

void loadFile () {
    FILE* file = fopen("PONG.ch8", "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file\n");
        return;
    }

    // file size
    fseek(file, 0, SEEK_END); // file pointer to the end
    size_t file_size = ftell(file); // get the position
    fseek(file, 0, SEEK_SET); // return to the starting position

    size_t result = fread(memory + 0x200, 1, sizeof(memory) - 0x200, file);
}

void loadFontSet() {
    for (int i = 0; i < 80; i++) {
        memory[0x50 + i] = chip8_fontset[i];
    }
}

void emulateCycle() {
    drawFlag = false;
    uint16_t opcode = memory[PC]; // why 16? 
    PC += 2;
    printf("Opcode: 0x%04X\n", opcode);
    
    // extraction
    uint16_t nnn = (opcode & 0xFFF);
    uint16_t n = (opcode & 0xF);
    uint16_t x = ((opcode >> 8) & 0xF);
    uint16_t y = ((opcode >> 4) & 0xF);
    uint16_t kk = (opcode & 0xFF);
    uint16_t temp = opcode & 0xF;
    uint16_t temp2 = opcode & 0xFF;
    uint16_t VF;
    uint16_t result;

    // decode
    switch (opcode & 0xF000) {
        case (0x0000):
            switch (temp2) {
                case (0x00E0):
                    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
                        screen[i] = 0;
                    }
                    break;
                case (0x00EE):
                    PC = stack[SP];
                    SP--;
                    break;
            }
            break;
        case (0x1000):
            PC = opcode & 0x0FFF;
            break;
        case (0x2000):
            SP++;
            stack[SP] = PC;
            PC = opcode & 0x0FFF;
            break;
        case (0x3000):
            if (reg[x] == kk) {
                PC += 2;
            }
            break;
        case (0x4000):
            if (reg[x] != kk) {
                PC += 2;
            }
            break;
        case (0x5000):
            if (reg[x] == reg[y]) {
                PC += 2;
            }
            break;
        case (0x6000):
            reg[x] = kk;
            break;
        case (0x7000):
            reg[x] = reg[x] + kk;
            break;
        case (0x8000):
            switch (temp) {
                case (0x0000):
                    reg[x] = reg[y];
                    break;
                case (0x0001):
                    reg[x] = reg[x] || reg[y];
                    break;
                case (0x0002):
                    reg[x] = reg[x] && reg[y];
                    break;
                case (0x0003):
                    reg[x] = reg[x] ^ reg[y];
                    break;
                case (0x0004):
                    result = reg[x] + reg[y];
                    if (result < reg[x] || result < reg[y]) {
                        reg[0xF] = 1;
                    }
                    else {
                        reg[0xF] = 0;
                    }
                    reg[x] = result;
                    break;
                case (0x0005):
                    result = reg[x] - reg[y];
                    if (reg[x] > reg[y]) {
                        reg[0xF] = 1;
                    }
                    else {
                        reg[0xF] = 0;
                    }
                    break;
                case (0x0006):
                    reg[x] = reg[x] >> 1;
                    reg[0xF] = reg[x] & 0x1;
                    break;
                case (0x0007):
                    reg[x] = reg[y] - reg[x];
                    if (reg[y] > reg[x]) {
                            reg[0xF] = 1;
                    }
                    else {
                        reg[0xF] = 0;
                    }
                    break;
                case (0x0008):
                    reg[0xF] = reg[x] << 1;
                    reg[0xF] = reg[x] & 0x1;
                    break;
                case (0x0009):
                    if (reg[x] != reg[y]) {
                        PC += 2;
                    }
                    break;
            }
            break;
        case (0xA000):
            I = nnn;
            break;
        case (0xB000):
            PC = nnn + reg[0x0];
            break;
        case (0xC000):
            reg[x] = rand() & 0xff;
            break;
        case (0xD000):
            uint16_t height = temp;
            uint8_t pixel;
            reg[0xF] = 0;
            	for (int yline = 0; yline < height; yline++) {
				    pixel = memory[I + yline];
                    for(int xline = 0; xline < 8; xline++) {
                        if((pixel & (0x80 >> xline)) != 0) {
                            if(screen[(reg[x] + xline + ((reg[y] + yline) * SCREEN_WIDTH))] == 1){
                                reg[0xF] = 1;                                   
                            }
                            screen[reg[x] + xline + ((reg[y] + yline) * SCREEN_WIDTH)] ^= 1;
                        }
				    }
			    }
            screen[10 * SCREEN_WIDTH + 10] = true; // Set a pixel at position (10, 10)
            screen[20 * SCREEN_WIDTH + 30] = true; // Set a pixel at position (30, 20)
            printf("test");
            drawFlag = true;
            break;
        case (0xE000):
            switch (temp2){
                case (0x009E):
                    if (keyboard[reg[x]]) { // is key pressed down?
                    PC += 2;
                    break;
                }
                case (0x00A1):
                    if (!keyboard[reg[x]]) {
                        PC += 2;
                    }
                    break;
            }
            break;
        case (0xF000):
            switch (temp2){
                case (0x0007):
                    reg[x] = delay;
                    break;
                case (0x000A):
                    for (int i = 0; i < 16; i++) {
                        if (keyboard[i]) {
                            reg[x] = i;
                        }
                    }
                    break;
                case (0x0015):
                    delay = reg[x];
                    break;
                case (0x0018):
                    sound = reg[x];
                    break;
                case (0x001E):
                    I = I + reg[x];
                    break;
                case (0x0029):
                    I = reg[x] * 5; // fix this wtf this mean??
                    break;
                case (0x0033):
                    char hex_str[20];
                    long dec_num;
                    sprintf(hex_str, "%X", opcode);
                    dec_num = strtol(hex_str, NULL, 16);
                    memory[I] = dec_num / 100;
                    memory[I + 1] = (dec_num % 100) / 10;
                    memory[I + 2] = dec_num % 10;
                    break;
                case (0x0055):
                    for (int i = 0; i < 16; i++) {
                        memory[I + i] = reg[i]; // might wanna change the 0xF thing
                    }
                    break;
                case (0x0065):
                    for (int i = 0; i < 16; i++) {
                        reg[i] = memory[I + i]; // might wanna change the 0xF thing
                    }
                    break;
            }
            break;
    }
    if (delay > 0) delay--;
    if (sound > 0) {
        sound--;
        //printf("beeeeeep");
    }
    PC += 2;
}

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
        SDL_PumpEvents();
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
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