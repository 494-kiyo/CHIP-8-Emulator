#include<stdio.h>
#include <stdint.h>

#define MEMORY_SIZE 4096
#define MEMORY_START 0x200

typedef struct chip8 {
    uint8_t registers[16]{};
    uint8_t SP = 0xfa0;
    uint16_t IR;
    uint16_t PC = 0x200;
    uint16_t stack[16]{};
    uint8_t delay;
    uint8_t sound;
    uint8_t *screen = 0xF00;
} chip8;

void chip8::emulateCycle() {
    uint16_t opcode;


}

int main() {
    printf("hello");
    return 0;
}