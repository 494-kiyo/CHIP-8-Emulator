all:
	gcc -Isrc/include -Lsrc/lib -o main main.c chip8.c -lmingw32 -lSDL2main -lSDL2
