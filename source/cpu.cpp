#include <cpu.h>
#include <ctime>
#include <filesystem>
#include <unistd.h>

int realKeyboard[] = {
    KEY_ZERO,   KEY_ONE,    KEY_TWO,    KEY_THREE,
    KEY_Q,      KEY_W,      KEY_E,      KEY_R,
    KEY_A,      KEY_S,      KEY_D,      KEY_F,
    KEY_Z,      KEY_X,      KEY_C,      KEY_V
};

int emulatorKeyboard[] = {
    0x01,   0x02,   0x03,   0x0C,
    0x04,   0x05,   0x06,   0x0D,
    0x07,   0x08,   0x09,   0x0E,
    0x0A,   0x00,   0x0B,   0x0F
};

bool updateVideo = false;

Cpu::Cpu()
{

    for (int i = 0; i < 0x1000; i++) {
        memory[i] = 0;
    }

    for (int i = 0; i < 2048; i++) {
        frameBuffer[i] = 0;
    }

    unsigned char chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i  = 0; i < 80; i++) {
        memory[0x50 + i] = chip8_fontset[i];
    }

    PC = 0x200;
}

int Cpu::upload_file_to_ram(const char *path) 
{

    FILE *fp = fopen(path, "rb");
    std::filesystem::path p{path};
    fread(&memory[0x200], 1, (size_t)std::filesystem::file_size(p), fp);
    fclose(fp);

    return 0;
}

void Cpu::process_insn(uint16_t op)
{

    uint8_t insn = (op >> 12) & 0xF;

    if (insn == 0) {
        switch (op) {
            case 0x00E0: // cls - clear the display
                ClearBackground(BLACK);
                break;    
            case 0x00EE: // return from subroutine
                PC = stack[SP];
                SP -= 1;
                break;
        }
    } else if (insn == 1) {
        // 1NNN
        // only one opcode starting with 1, which is jump, jumps to nnn
        PC = op & 0xFFF;

    } else if (insn == 2) {
        // 2NNN
        // only one opcode starting with 2, which is calling subroutine at nnn
        uint16_t subroutine_addr = op & 0xFFF;
        
        ++SP;
        stack[SP] = PC;
        PC = subroutine_addr;

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

        V[x_reg] = (uint8_t)(op & 0xFF);

    } else if (insn == 7) {
        // 7XNN
        // only one opcdoe starting with 7, adding nn to vx

        uint8_t x_reg = (op >> 8) & 0xF;

        V[x_reg] += (uint8_t)(op & 0xFF);

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
                V[0xF] = 1;
            } else {
                V[0xF] = 0;
            }

        } else if (insn_kind == 5) {
            // 8XY5
            // VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not. 

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] -= V[y_reg];

            if (V[x_reg] - V[y_reg] < 0) {
                V[0xF] = 0;
            } else {
                V[0xF] = 1;
            }

        } else if (insn_kind == 6) {
            // 8XY6
            // Stores the least significant bit of VX in VF and then shifts VX to the right by 1
            uint8_t x_reg = (op >> 8) & 0xF;
            V[0xF] = V[x_reg] & 0x1;
            V[x_reg] >>= 1;

        } else if (insn_kind == 7) {
            // 8XY7
            // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not. 

            uint8_t x_reg = (op >> 8) & 0xF;
            uint8_t y_reg = (op >> 4) & 0xF;

            V[x_reg] = V[y_reg] - V[x_reg];

            if (V[y_reg] - V[x_reg] < 0) {
                V[0xF] = 0;
            } else {
                V[0xF] = 1;
            }
        } else if (insn_kind == 0xE) {
            // 8XYE
            // Stores the most significant bit of VX in VF and then shifts VX to the left by 1.

            uint8_t x_reg = (op >> 8) & 0xF;
            V[0xF] = (V[x_reg] >> 7) & 0x1;
            V[x_reg] <<= 1;
        }

    } else if (insn == 9) {

        uint8_t y_reg = (op >> 4) & 0xF;
        uint8_t x_reg = (op >> 8) & 0xF;

        if (V[y_reg] != V[x_reg]) {
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

        uint8_t x_reg = (op >> 8) & 0xF;
        uint8_t y_reg = (op >> 4) & 0xF;
        uint8_t n = op & 0xF;

        V[0xF] = 0;

        for (int i = 0; i < n; i++) {

            uint8_t pixelLine = memory[I + i];

            for (int j = 0; j < 8; j++) {

                if (pixelLine & (0x80 >> j)) {

                    if (frameBuffer[(V[x_reg] + j + ((V[y_reg] + i) * 64))] == 1) {
                        V[0xF] = 1;
                    } 
                    frameBuffer[V[x_reg] + j + ((V[y_reg] + i) * 64)] ^= 1;
                }
            }
        }

        updateVideo = true;

    } else if (insn == 0xE) {

        uint8_t insn_kind = op & 0xFF;

        if (insn_kind == 0x9E) {
            // EX9E
            // Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block); 

            uint8_t x_reg = (op >> 8) & 0xF;

            for (int i = 0; i < 0x0F; i++) {
                if (IsKeyDown(realKeyboard[i])) {   
                    if (V[x_reg] == emulatorKeyboard[i]) {
                        PC += 2;
                    }
                }
            }

        } else if (insn_kind == 0xA1) {
            // EXA1
            // Skips the next instruction if the key stored in VX is not pressed. (Usually the next instruction is a jump to skip a code block); 
            uint8_t x_reg = (op >> 8) & 0xF;

            for (int i = 0; i < 0x0F; i++) {
                if (IsKeyUp(realKeyboard[i])) {   
                    if (V[x_reg] == emulatorKeyboard[i]) {
                        PC += 2;
                    }
                }
            }
        }
    } else if (insn == 0xF) {

        uint8_t insn_kind = op & 0xFF;

        if (insn_kind == 0x07) {
            // FX07
            // Sets VX to the value of the delay timer. 

            uint8_t x_reg = (op >> 8) & 0xF;

            V[x_reg] = delayTimer;

        } else if (insn_kind == 0x0A) {
            // FX0A
            // A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event); 

            uint8_t x_reg = (op >> 8) & 0xF;

            keyPressed = false;

            for (int i = 0; i < 0x0F; i++) {
                if (GetKeyPressed() == realKeyboard[i]) {
                    V[x_reg] = emulatorKeyboard[i];
                    keyPressed = true;
                }
            }

            if (!keyPressed) {
                PC -= 2;
            }

        } else if (insn_kind == 0x15) {
            // FX15
            // Sets the delay timer to VX.
            uint8_t x_reg = (op >> 8) & 0xF;
            delayTimer = V[x_reg];

        } else if (insn_kind == 0x18) {
            // FX18
            // Sets the sound timer to VX. 
            uint8_t x_reg = (op >> 8) & 0xF;
            soundTimer = V[x_reg];

        } else if (insn_kind == 0x1E) {
            // FX1E
            // Adds VX to I. VF is not affected.
            uint8_t x_reg = (op >> 8) & 0xF;
            I += V[x_reg];


        } else if (insn_kind == 0x29) {
            // FX29
            // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. 

            I = 0x50 + ((op >> 8) & 0xF) * 5;

        } else if (insn_kind == 0x33) {
            // FX33
            // Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, 
            // the middle digit at I plus 1, and the least significant digit at I plus 2. 
            // (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, 
            // the tens digit at location I+1, and the ones digit at location I+2.); 

            // FIXME: test_opcode rom says this insn isn't working properly

            uint8_t x_reg = (op >> 8) & 0xF;

            memory[I] = (uint8_t)(V[x_reg] / 100);
            memory[I+1] = (V[x_reg] / 10) % 10;
            memory[I+2] = (V[x_reg]) % 10;

        } else if (insn_kind == 0x55) {
            // FX55
            // Stores V0 to VX (including VX) in memory starting at address I. 
            // The offset from I is increased by 1 for each value written, but I itself is left unmodified.

            uint8_t x_reg = (op >> 8) & 0xF;

            uint16_t mem_addr = I;

            for (int i = 0; i <= x_reg; i++) {
                memory[mem_addr] = V[i];
                mem_addr += 1;
            }

        } else if (insn_kind == 0x65) {
            // FX65
            // Fills V0 to VX (including VX) with values from memory starting at address I. 
            // The offset from I is increased by 1 for each value written, but I itself is left unmodified.

            uint8_t x_reg = (op >> 8) & 0xF;

            uint16_t mem_addr = I;

            for (int i = 0; i <= x_reg; i++) {
                V[i] = memory[mem_addr];
                mem_addr += 1;
            }
        }
    }
}

void Cpu::step() 
{

    uint16_t insn = memory[PC] << 8 | memory[PC+1];

    PC += 2;

    if (delayTimer > 0) {
        delayTimer--;
    }
    
    if (soundTimer > 0) {
        soundTimer--;
    }
    
    process_insn(insn);
}

void Cpu::run() 
{

    InitWindow(640, 320, "raylib");
    
    ClearBackground(BLACK);

    SetTargetFPS(500);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        step();

        if (updateVideo) {
            updateVideo = false;

            for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 64; x++) {
                    if (frameBuffer[(y * 64) + x] != 0) {
                        DrawRectangle (x * 10, y * 10, 10, 10, WHITE);
                    } else {
                        DrawRectangle (x * 10, y * 10, 10, 10, BLACK);
                    }
                }
            }
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}