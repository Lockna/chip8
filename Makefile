all:
	g++ source/*.cpp -o chip8 -g -lSDL2 -Wall -Werror -fsanitize=address -fsanitize=undefined -Iinclude/chip8/*
	@ ./chip8