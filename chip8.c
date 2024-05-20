#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"

uint16_t keyboard[16] = {0};
uint8_t reg[16] = {0}; // 16 general purpose registers
uint8_t SP = 0; // stack pointer
uint16_t I = 0; // current address
uint16_t PC = 0x200; // next address
uint16_t stack[16] = {0}; // stack
uint8_t memory[MEMORY_SIZE] = {0};
uint8_t delay = 0;
uint8_t sound = 0;
bool drawFlag = false;
bool screen [SCREEN_WIDTH * SCREEN_HEIGHT] = {false};

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

    size_t result = fread(memory + 0x200, sizeof(uint16_t), file_size, file);
}

void loadFontSet() {
    memcpy(memory, chip8_fontset, sizeof(chip8_fontset));
}


void emulateCycle() {
    uint16_t opcode = memory[PC] << 8 | memory[PC + 1]; // why 16? 
    PC += 2;
    printf("Opcode: 0x%04X\n", opcode);
    
    // extraction
    uint16_t nnn = (opcode & 0x0FFF);
    uint8_t n = (opcode & 0x000F);
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = (opcode & 0x00FF);
    uint8_t temp = opcode & 0x000F;
    uint8_t temp2 = opcode & 0x00FF;
    int result;

    // decode
    switch (opcode & 0xF000) {
        case (0x0000):
            switch (kk) {
                case (0x00E0):
                    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
                        screen[i] = false;
                    }
                    drawFlag = true;
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
                    reg[x] = reg[x] | reg[y];
                    break;
                case (0x0002):
                    reg[x] = reg[x] & reg[y];
                    break;
                case (0x0003):
                    reg[x] = reg[x] ^ reg[y];
                    break;
                case (0x0004):
                    result = (int)reg[x] + (int)reg[y];
                    if (result > 255) {
                        reg[0xF] = 1;
                    }
                    else {
                        reg[0xF] = 0;
                    }
                    reg[x] = reg[x] + reg[y];
                    break;
                case (0x0005):
                    if (reg[x] > reg[y]) {
                        reg[0xF] = 1;
                    }
                    else {
                        reg[0xF] = 0;
                    }
                    reg[x] = reg[x] - reg[y];
                    break;
                case (0x0006):
                    reg[0xF] = reg[x] & 0x1;
                    reg[x] = reg[x] >> 1;
                    break;
                case (0x0007):
                    if (reg[y] > reg[x]) {
                            reg[0xF] = 1;
                    }
                    else {
                        reg[0xF] = 0;
                    }
                    reg[x] = reg[y] - reg[x];
                    break;
                /*case (0x0008):
                    reg[0xF] = reg[x] << 1;
                    reg[0xF] = reg[x] & 0x1;
                    break;
                case (0x0009):
                    if (reg[x] != reg[y]) {
                        PC += 1;
                    }
                    break; */
                case (0x000E):
                    reg[0xF] = reg[x] >> 7;
                    reg[x] <<= 1;
                    break;
            }
            break;
        case (0x9000):
            if (reg[x] != reg[y]) {
                PC += 2;
            }
        case (0xA000):
            I = nnn;
            break;
        case (0xB000):
            PC = (nnn) + reg[0x0];
            break;
        case (0xC000):
            reg[x] = (rand() % 0x100) & (kk);
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
            drawFlag = true;
            break;
        case (0xE000):
            switch (kk){
                case (0x009E):
                    if (keyboard[reg[x]] != 0) { // is key pressed down?
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
            switch (kk){
                case (0x0007):
                    reg[x] = delay;
                    break;
                case (0x000A):
                //int pressed = 0;
                    for (int i = 0; i < 16; i++) {
                        if (keyboard[i]) {
                            reg[x] = i;
                            PC += 2;
                            break;
                        }
                        //if (pressed == 0) {
                            //PC -= 2;
                        //}
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
                    //char hex_str[20];
                    //long dec_num;
                    //sprintf(hex_str, "%X", opcode);
                    //dec_num = strtol(hex_str, NULL, 16);
                    memory[I] = (reg[x] % 1000) / 100;
                    memory[I + 1] = (reg[x] % 100) / 10;
                    memory[I + 2] = reg[x] % 10;
                    break;
                case (0x0055):
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = reg[i]; // might wanna change the 0xF thing
                    }
                    break;
                case (0x0065):
                    for (int i = 0; i <= x; i++) {
                        reg[i] = memory[I + i]; // might wanna change the 0xF thing
                    }
                    break;
            }
            break;
        default:
            printf("---ERROR--- Opcode: 0x%04X\n", opcode);
    }
    if (delay > 0) delay--;
    if (sound > 0) {
        sound--;
        printf("beeeeeep");
    }
}