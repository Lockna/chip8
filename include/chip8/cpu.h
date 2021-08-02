#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <iostream>

class Cpu {

public:
    Cpu();
    ~Cpu() = default;
    int upload_file_to_ram(std::string path);
    void run();
    void step();
    uint16_t fetch_insn();
    void process_insn(uint16_t op);

private:
    uint8_t V[16];
    uint8_t memory[4096];
    uint16_t stack[16];
    uint16_t I;
    uint16_t PC;
    uint8_t SP;
    
    uint8_t* screen_buffer;

};

#endif
