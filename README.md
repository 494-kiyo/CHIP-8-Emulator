# A Complete CHIP-8 Emulator in C using SDL2
This is a program that emulates [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) games. This was completely written in C using the SDL2 library to show the display.

## Specification
- Memory has 4KB of RAM and memory locations are 8 bits. The actual program (game) is loaded in at location 512 (0x200) as the first 512 bytes are used for fontset and other information
- It has 16 8-bit registers. HEX is used to address the registers and the last register register 0xF is used as a flag
- Stack has 16 levels to store return addresses
- There are two timers; one for the delay and one for the sound. Both are used to count 60 Hz
- The input uses a hex keyboard and thus, a font set is loaded in to allow translation from modern input
- The display is 64x32 pixels and this program uses a scale of 8

## Example (Ping Pong)
![image](https://github.com/494-kiyo/CHIP-8-Emulator/assets/81718559/161b78b6-fee0-4322-b5ea-168a974db02e)
