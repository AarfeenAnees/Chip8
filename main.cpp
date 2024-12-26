#include <iostream>

#include "raylib.h"

#include "chip8.h"


int main()
{
	Chip8 chip8;
	chip8.load_rom("A:\\C++\\Projects\\CHIP-8\\ROMs\\4-flags.ch8");

	SetConfigFlags(0);  // Disable vsync and any other flags
	InitWindow(Chip8::columns*Chip8::upscale_factor, Chip8::rows*Chip8::upscale_factor, "CHIP-8");

	//ClearBackground(off);

	

	size_t instruction_count{1};
	
	

	while (WindowShouldClose() == false)
	{
		chip8.increment_pc(); //sets current_Opcode, and increments pc by 2

		switch (chip8.get_opcode() & 0xf000) //extract the first nibble
		{
			case 0x0000:
				switch (chip8.get_opcode() & 0x000f)
				{
					case 0x0000:
						chip8.OP_00E0();
						break;
					case 0x000E:
						chip8.OP_00EE();
						break;
				}
				break;

			case 0x1000:
				chip8.OP_1NNN();
				break;

			case 0x2000:
				chip8.OP_2NNN();
				break;

			case 0x3000:
				chip8.OP_3XNN();
				break;

			case 0x4000:
				chip8.OP_4XNN();
				break;

			case 0x5000:
				chip8.OP_5XY0();
				break;

			case 0x6000:
				chip8.OP_6XNN();
				break;

			case 0x7000:
				chip8.OP_7XNN();
				break;

			case 0x8000:
				switch (chip8.get_opcode() & 0x000f)
				{
					case 0x0000:
						chip8.OP_8XY0();
						break;
					case 0x0001:
						chip8.OP_8XY1();
						break;
					case 0x0002:
						chip8.OP_8XY2();
						break;
					case 0x0003:
						chip8.OP_8XY3();
						break;
					case 0x0004:
						chip8.OP_8XY4();
						break;
					case 0x0005:
						chip8.OP_8XY5();
						break;
					case 0x0006:
						chip8.OP_8XY6();
						break;
					case 0x0007:
						chip8.OP_8XY7();
						break;
					case 0x000E:
						chip8.OP_8XYE();
						break;
				}
				break;

			case 0xA000:
				chip8.OP_ANNN();
				break;

			case 0xD000:
				chip8.OP_DXYN();
				break;

			case 0xF000:
				switch (chip8.get_opcode() & 0x00ff)
				{
					case 0x01E:
						chip8.OP_FX1E();
						break;
					case 0x033:
						chip8.OP_FX33();
						break;
					case 0x055:
						chip8.OP_FX55();
						break;
					case 0x065:
						chip8.OP_FX65();
						break;
				}
				break;
		}
		
		


		if (!(instruction_count % /*Clock::instructions_per_frame*/ 11))
		{
			
			chip8.update_screen();
			

			
		}
		
		if (++instruction_count == 110) break;
		
	}

	CloseWindow();
}