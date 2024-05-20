#include <stdint.h>

#define MEMORY_SIZE 4096
#define MEMORY_START 0x200

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SCALE 8

#define OPCODE_X(opcode) ((opcode) & 0xF)

extern uint16_t keyboard[16];
extern bool screen [SCREEN_WIDTH * SCREEN_HEIGHT];
extern bool drawFlag;

void loadFile(void);
void loadFontSet(void);
void emulateCycle(void);