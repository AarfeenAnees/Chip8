#include "chip8.h"


Color off = DARKPURPLE;
Color on = ORANGE;




Chip8::Chip8()
{

	uint16_t start = 0x200;
	pc = start;						//setting pc to first instruction at 0x200

	sp = 0;						   //initially, stack is empty and sp points to index 0 in stack, ready to push the first return address here


	uint16_t font_start = 0x50;		//loading fonts into memory
	for (int i = 0; i < fontset_size; i++)
	{
		memory[font_start + i] = font[i];
	}


}

void Chip8::increment_pc()
{
	//OPcode constitutes the conjugated 16 bit value from pc and pc+1. As memory is addressed in bytes, we need to fetch both bytes individually and then join them.
	current_opcode = memory[pc] << 8 | memory[pc + 1];

	pc += 2;					//increment pc by 2 before executing instructions.
}

/************************************TEST STUFF**************************************/

int run_count{1};
std::fstream log_draw("C:\\Users\\anees\\Desktop\\log_updateScreen.txt", std::fstream::out);

std::chrono::high_resolution_clock clock_;

/************************************TEST STUFF**************************************/

void Chip8::update_screen()
{

	log_draw << "RUN NUMBER" << run_count << "\n";

	if (!IsWindowReady())
	{
		std::cerr << "Window Is Not Open!";
		std::terminate();
	}

	BeginDrawing();

	ClearBackground(off);
	

	for (int i = 0; i < rows; i++)	  //32 rows i.e i [0,32) and represents y
	{
		for (int j = 0; j < columns; j++)  //64 columns i.e j [0,64) and represents x
		{
			Color pixel_state = ((display[i] >> j) & 0b1) ? on : off;     //extracts j'th bit from last
			DrawRectangle((columns - (j + 1)) * upscale_factor, i * upscale_factor, upscale_factor, upscale_factor, pixel_state);
			
		}
	}
	
	auto start = clock_.now();
	EndDrawing();
	while (std::chrono::duration_cast<std::chrono::microseconds>(clock_.now() - start) < std::chrono::microseconds{ 2000 }) {}
	log_draw << "End Drawing :" << std::chrono::duration_cast<std::chrono::microseconds>(clock_.now() - start) << "\n" << std::endl;
	++run_count;
}

uint16_t Chip8::get_opcode()
{
	return current_opcode;
}

//Clock& Chip8::getClock()
//{
//	return clock;
//}

void Chip8::OP_00E0()
{
	memset(display, 0, sizeof(display));
}

void Chip8::OP_00EE()
{
	pc = stack[--sp];	//after subroutine ends, pc is set to return address to continue execution
}

void Chip8::OP_1NNN()
{
	pc = current_opcode & 0x0fff; //pc = NNN
}

void Chip8::OP_2NNN()
{
	stack[sp++] = pc;	//store the return address of subroutine in stack, and move sp to next empty position in stack.
	uint16_t subroutine_addr = current_opcode & 0x0fff;
	pc = subroutine_addr;   //assign subroutine address to pc.
}

void Chip8::OP_3XNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t nn = (current_opcode & 0x00ff);
	registers[vx] == nn ? pc += 2 : NULL;
}

void Chip8::OP_4XNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t nn = (current_opcode & 0x00ff);
	registers[vx] != nn ? pc += 2 : NULL;
}

void Chip8::OP_5XY0()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0);
	registers[vx] == registers[vy] ? pc += 2 : NULL;
}

void Chip8::OP_6XNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	registers[vx] = current_opcode & 0x00ff;									//VX = NN
}

void Chip8::OP_7XNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	registers[vx] += current_opcode & 0x00ff;									//VX += NN
}

void Chip8::OP_8XY0()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] = registers[vy];
}

void Chip8::OP_8XY1()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] |= registers[vy];

}

void Chip8::OP_8XY2()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] &= registers[vy];
}

void Chip8::OP_8XY3()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] ^= registers[vy];
}

void Chip8::OP_8XY4()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] + registers[vy] > 255 ? registers[0xF] = 1 : registers[15] = 0; //VF aka carry flag set to 1 if VX+VY causes overflow, else set to 0
	registers[vx] += registers[vy];
}

void Chip8::OP_8XY5()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] > registers[vy] ? registers[0xF] = 1 : registers[15] = 0; //VF set to 1 if minuend (right operand) is larger, else 0 if subtrahend is larger
	registers[vx] = registers[vx] - registers[vy];
}

void Chip8::OP_8XY6()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] = registers[vy];				//set vX to vY
	registers[0xF] = registers[vx] & 0x01;		//... -(2) and set vF to the shifted out bit
	registers[vx] >>= 1;						//    -(1) shift VX to the right by one bit  ...
}

void Chip8::OP_8XY7()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] > registers[vy] ? registers[15] = 1 : registers[15] = 0; //VF set to 1 if minuend (right operand) is larger, else 0 if subtrahend is larger
	registers[vx] = registers[vy] - registers[vx];
}

void Chip8::OP_8XYE()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	registers[vx] = registers[vy];						//set vX to vY
	registers[0xF] = (registers[vx] >> 7) & 0x01;		//... -(2) and set vF to the shifted out bit
	registers[vx] <<= 1;								//    -(1) shift VX to the left by one bit  ...
}

void Chip8::OP_9XY0()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0);
	registers[vx] != registers[vy] ? pc += 2 : NULL;
}

void Chip8::OP_ANNN()
{
	index = current_opcode & 0x0fff;											//index = NNN
}

void Chip8::OP_BNNN()
{
	pc = (current_opcode & 0x0fff) + registers[0];
}

void Chip8::OP_CXNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t nn = (current_opcode & 0x00ff);

	registers[vx] = nn & static_cast<uint8_t>(distribution(generator));
}

void Chip8::OP_DXYN()
{

	//coordinate x,y of (left) corner bit of sprite
	uint8_t x = registers[(current_opcode & 0x0f00) >> 8] % columns;			//X
	uint8_t y = registers[(current_opcode & 0x00f0) >> 4] % rows;				//Y
	uint8_t sprite_height = current_opcode & 0x000f;							//N

	registers[15] = 0;															//set VF = 0 if no pixels have been turned off (start with this assumption)

	for (uint8_t row = y; row < y + sprite_height; row++)
	{
		if (row >= rows) break;													//clipping condition for height.
		uint64_t display_row = display[row];
		uint64_t sprite_row = static_cast<uint64_t>(memory[index]);				//fetch the 8 bit sprite, and place it on 64 bit long row (sprite first bit at 56th bit)        
		sprite_row = (x >= 56) ? sprite_row >> (x - 56) : sprite_row << (56 - x);                  //sprite occupies 56th to 64th bit. Move it to xth place on the row. Also comes with width clipping as bonus.

		if (!(display_row & (display_row ^ sprite_row))) registers[15] = 1;	//set VF = 1 if display pixel has been turned off  
		display[row] = display_row ^ sprite_row;
		index++;																//move to the next 8 bits.
	}


}

void Chip8::OP_FX1E()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	index += registers[vx];
}

void Chip8::OP_FX33()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;

	uint8_t value = registers[vx];

	for (int i = 2; i >= 0; i--)
	{
		memory[index + i] = value % 10;
		value /= 10;
	}
}

void Chip8::OP_FX55()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;

	for (int i = 0; i <= vx; i++)
	{
		memory[index++] = registers[i];
	}
	//index is index+vx+1 now
}

void Chip8::OP_FX65()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;

	for (int i = 0; i <= vx; i++)
	{
		registers[i] = memory[index++];
	}
	//index is index+vx+1 now
}


void Chip8::load_rom(const char* rom_path)
{
	std::ifstream rom(rom_path, std::ios::binary | std::ios::ate); //open file in binary format, and place the file pointer at end of file

	if (rom.is_open())
	{

		std::streampos size{ rom.tellg() };				//tellg returns the "distance" of file pointer's current position (which is currently at the end) from start in bytes
		rom.seekg(0, std::ios::beg);						//Reset to the beginning of the file
		std::vector<char> buffer(size);						//buffer to read contents of rom. vector prefered over dynamic raw array due to RAII

		std::filebuf* rom_buf{ rom.rdbuf() };				//get the buffer assocoiated with the file
		rom_buf->sgetn(buffer.data(), size);				//dump the contents of file buffer into our buffer (sgetn == "stream get n", where n is number of bytes)

		uint16_t start = 0x200;								//tranfer contents of buffer to memory
		for (int i = 0; i < static_cast<int>(size); i++)
		{
			memory[start + i] = static_cast<uint8_t>(buffer[i]);
		}
	}
	else
	{
		std::cerr << "Failed to open file" << std::endl;
	}

	//rom.close();											redundant as destructor closes the file at end of scope
}