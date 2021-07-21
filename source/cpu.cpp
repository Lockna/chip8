#include <cpu.h>
#include <ctime>

Cpu::Cpu()
{
    screen_buffer = &memory[0xF00];
    PC = 0x200;
    SP = 0xFA0;
}

int Cpu::upload_file_to_ram(std::string path) 
{

}

uint16_t Cpu::fetch_insn()
{
    return memory[PC];
}

void Cpu::process_insn(uint16_t op)
{

    uint8_t insn = op >> 12;

    if (insn == 0) {
        switch (op) {
            case 0x00E0: // cls - clear the display
                break;    
            case 0x00EE: // return from subroutine
                PC = memory[SP];
                SP -= 1;
                break;
        }
    } else if (insn == 1) {
        // 1NNN
        // only one opcode starting with 1, which is jump, jumps to nnn
        SP += 1;
        memory[SP] = PC;
        PC = op & 0xFFF;

    } else if (insn == 2) {
        // 2NNN
        // only one opcode starting with 2, which is calling subroutine at nnn
        uint16_t subroutine_addr = op & 0xFFF;
        PC = memory[subroutine_addr];

    } else if (insn == 3) {
        // 3XNN
        // only one opcode starting with three, skip next insn if vx equals nn
        uint8_t compare_value = op & 0xFF;
        uint8_t reg = (op >> 8) & 0xF;

        if (compare_value == V[reg]) {
            PC += 2;
        }

    } else if (insn == 4) {
        // 4XNN
        // only one opcode starting with 4, skip next insn if vx not equals nn

        uint8_t compare_value = op & 0xFF;
        uint8_t reg = (op >> 8) & 0xF;

        if (compare_value != V[reg]) {
            PC += 2;
        }

    } else if (insn == 5) {
        // 5XY0
        // only one opcode starting with 5, skip next instruction if vx equals vy

        uint8_t y_reg = (op >> 4) & 0xF;
        uint8_t x_reg = (op >> 8) & 0xF;

        if (y_reg == x_reg) {
            PC += 2;
        }

    } else if (insn == 6) {
        // 6XNN
        // only one opcode starting with 6, setting vx to nn

        uint8_t x_reg = (op >> 8) & 0xF;

        V[x_reg] = op & 0xFF;

    } else if (insn == 7) {
        // 7XNN
        // only one opcdoe starting with 7, adding nn to vx

        uint8_t x_reg = (op >> 8) & 0xF;

        V[x_reg] += op & 0xFF;

    } else if (insn == 8) {

        uint8_t insn_kind = op & 0xF;

        if (insn_kind == 0) {
            // 8XY0
            // sets vx to the value of vy

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] = V[y_reg];

        } else if (insn_kind == 1) {
            // 8XY1
            // Sets VX to VX or VY. (Bitwise OR operation); 

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] = V[x_reg] | V[y_reg];

        } else if (insn_kind == 2) {
            // 8XY2
            // Sets VX to VX and VY. (Bitwise AND operation); 

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] = V[x_reg] & V[y_reg];

        } else if (insn_kind == 3) {
            // 8XY3
            // Sets VX to VX xor VY.

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] = V[x_reg] ^ V[y_reg];

        } else if (insn_kind == 4) {
            // 8XY4
            // Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not. 

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] = V[x_reg] + V[y_reg];

            if (V[x_reg] + V[y_reg] > 255) {
                VF = 1;
            } else {
                VF = 0;
            }

        } else if (insn_kind == 5) {
            // 8XY5
            // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not. 

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] -= V[y_reg];

            if (V[x_reg] - V[y_reg] < 0) {
                VF = 0;
            } else {
                VF = 1;
            }

        } else if (insn_kind == 6) {
            // 8XY6
            // Stores the least significant bit of VX in VF and then shifts VX to the right by 1
            uint8_t x_reg = (op >> 8) & 0xF;
            VF = V[x_reg] & 0x1;
            V[x_reg] >>= 1;

        } else if (insn_kind == 7) {
            // 8XY7
            // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not. 

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] = V[y_reg] - V[x_reg];

            if (V[y_reg] - V[x_reg] < 0) {
                VF = 0;
            } else {
                VF = 1;
            }
        } else if (insn_kind == 0xE) {
            // 8XYE
            // Stores the most significant bit of VX in VF and then shifts VX to the left by 1.

            uint8_t x_reg = (op >> 8) & 0xF;
            VF = (V[x_reg] >> 7) & 0x1;
            V[x_reg] <<= 1;
        }

    } else if (insn == 9) {

        uint8_t y_reg = (op >> 4) & 0xF;
        uint8_t x_reg = (op >> 8) & 0xF;

        if (y_reg != x_reg) {
            PC += 2;
        }

    } else if (insn == 0xA) {
        // ANNN
        // Sets I to the address NNN. 
        I = op & 0xFFF;
    }  else if (insn == 0xB) {
        // BNNN
        // Jumps to the address NNN plus V0. 
        uint16_t addr = op & 0xFFF;
        PC = V[0] + addr;

    } else if (insn == 0xC) {
        // CXNN
        // Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.

        uint8_t x_reg = (op >> 8) & 0xF;

        V[x_reg] = (rand() % 256) & (op & 0xFF);

    } else if (insn == 0xD) {
        // DXYN
        // Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels. 
        // Each row of 8 pixels is read as bit-coded starting from memory location I; 
        // I value does not change after the execution of this instruction. 
        // As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen 

        // draw(Vx, Vy, N);

    } else if (insn == 0xE) {

    } else if (insn == 0xF) {

    }

}

void Cpu::step() 
{

    uint16_t insn = fetch_insn();

    process_insn(insn);

    PC += 2;

}

void Cpu::run() 
{
    while (true) {
        step();
    }
}