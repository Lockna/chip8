#include <iostream>
#include <SDL2/SDL.h>
#include <cpu.h>
#include <ctime>

int main(int argc, char *argv[]) 
{

    // SDL_Init(SDL_INIT_EVERYTHING);

    // SDL_Window *window = SDL_CreateWindow(
    //     "CHIP8",
    //     SDL_WINDOWPOS_UNDEFINED,
    //     SDL_WINDOWPOS_UNDEFINED,
    //     64,
    //     32,
    //     SDL_WINDOW_SHOWN
    // );

    // if (window == nullptr) {
    //     std::cout << "Could not create window: " << SDL_GetError() << std::endl;
    // }

    srand(time(NULL));

    Cpu cpu;



}