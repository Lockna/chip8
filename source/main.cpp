#include <iostream>
#include <cpu.h>
#include <ctime>

int main(int argc, char *argv[]) 
{

    srand(time(NULL));

    Cpu cpu;

    cpu.upload_file_to_ram(argv[1]);

    cpu.run();

    return 0;
}
