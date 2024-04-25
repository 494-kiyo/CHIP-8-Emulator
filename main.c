#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define MEMORY_SIZE 4096
#define MEMORY_START 0x200

#define OPCODE_X(opcode) ((opcode) & 0xF)

uint8_t reg[16]; // 16 general purpose registers
uint8_t SP; // stack pointer
uint16_t I; // current address
uint16_t PC = 0x200; // next address
uint16_t stack[16]; // stack
uint16_t memory[MEMORY_SIZE];
uint8_t delay;
uint8_t sound;
uint8_t screen;

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

void emulateCycle() {
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
    switch (opcode & 0xF) {
        case (0x0000):
            switch (temp2) {
                case (0x00E0):
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
            if (reg[x] = kk) {
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
            break;
        case (0xE000):
            switch (temp2){
                case (0x009E):
                    if (reg[x] == 1) { // is key pressed down?
                    PC += 2;
                    break;
                }
                case (0x00A1):
                    if (reg[x] == 0) {
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
                    if (1) { // if key is pressed

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
                    I = reg[x]; // fix this
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
            }
    }
    if (delay > 0) delay--;
    if (sound > 0) {
        sound--;
        printf("beeeeeep");
    }
    PC += 2;
}

int main() {
    srand(time(NULL));
    loadFile();
    while (1){
        emulateCycle();
    }
    return 0;
}