#include<stdio.h>
#include <stdint.h>

#define MEMORY_SIZE 4096
#define MEMORY_START 0x200

#define OPCODE_X(opcode) ((opcode) & 0xF)

uint8_t registers[16]; // 16 general purpose registers
uint8_t SP; // stack pointer
uint16_t IR; // current address
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

}

int main() {
    loadFile();
    emulateCycle();
    return 0;
}