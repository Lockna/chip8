all:
	g++ source/*.cpp -o chip8 -g -Wall -Werror -fsanitize=address -fsanitize=undefined -Iinclude/chip8/ -lraylib
