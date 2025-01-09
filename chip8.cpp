#include "chip8.h"
#include <algorithm>

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

int run_count{ 1 };
std::fstream log_draw("C:\\Users\\anees\\Desktop\\log_updateScreen_46.txt", std::fstream::out);

std::chrono::high_resolution_clock clock_;

/************************************TEST STUFF**************************************/

void Chip8::update_screen()
{
	auto start = clock_.now();

	// create an image the same size as the window for us to draw too
	Image imageBuffer = GenImageColor(columns*upscale_factor, rows*upscale_factor, BLACK);

	// make a texture that matches the format and size of the image
	Texture displayTexture = LoadTextureFromImage(imageBuffer);

	for (int i = 0; i < rows; i++)	  //32 rows i.e i [0,32) and represents y
	{
		for (int j = 0; j < columns; j++)  //64 columns i.e j [0,64) and represents x
		{
			Color pixel_state = ((display[i] >> j) & 0b1) ? on : off;     //extracts j'th bit from last
			ImageDrawRectangle(&imageBuffer, (columns - (j + 1)) * upscale_factor, i * upscale_factor, upscale_factor, upscale_factor, pixel_state);
		
		}
	}

	UpdateTexture(displayTexture, imageBuffer.data);

	BeginDrawing();

	if (delay_timer) delay_timer--;
	if (sound_timer) sound_timer--;

	ClearBackground(WHITE);

	// display the texture from the GPU to the screen
	DrawTexture(displayTexture, 0, 0, WHITE);

	EndDrawing();
	log_draw << std::chrono::duration_cast<std::chrono::microseconds>(clock_.now() - start) << std::endl;

}

uint16_t Chip8::get_opcode()
{
	return current_opcode;
}


/*
[1] [2] [3] [4]						0x2   0x3   0x4   0x5						1 2 3 C
[Q] [W] [E] [R]   ==SCANCODES==>	0x10  0x11  0x12  0x13   ==CHIP8_KEYS==>	4 5 6 D
[A] [S] [D] [F]						0x1E  0x1F  0x20  0x21						7 8 9 E
[Z] [X] [C] [V]                     0x2C  0x2D  0x2E  0x2F						A 0 B F

But this wouldn't be needed, as raylibs keys are physical keys!

*/



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
	if (registers[vx] == nn) pc += 2;
}

void Chip8::OP_4XNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t nn = (current_opcode & 0x00ff);
	if (registers[vx] != nn) pc += 2; 
}

void Chip8::OP_5XY0()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;
	if (registers[vx] == registers[vy]) pc += 2;
}

void Chip8::OP_6XNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t nn = (current_opcode & 0x00ff);
	registers[vx] = nn;									//VX = NN
}

void Chip8::OP_7XNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t nn = (current_opcode & 0x00ff);
	registers[vx] += nn;									//VX += NN
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

	uint16_t sum = registers[vx] + registers[vy];
	constexpr uint8_t max = std::numeric_limits<uint8_t>::max(); //255

	registers[0xF] = sum > max ? 1 : 0; //VF aka carry flag set to 1 if VX+VY causes overflow, else set to 0
	registers[vx] = sum;
}

void Chip8::OP_8XY5()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t vy = (current_opcode & 0x00f0) >> 4;

	registers[0xF] = (registers[vx] > registers[vy] ? 1 : 0 ); //VF set to 1 if minuend (left operand) is larger, else 0 if subtrahend is larger
	registers[vx] -= registers[vy];
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

	registers[0xF] = (registers[vx] < registers[vy] ? 1 : 0); //VF set to 1 if minuend (right operand) is larger, else 0 if subtrahend is larger
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
	uint8_t vy = (current_opcode & 0x00f0) >> 4;

	if(registers[vx] != registers[vy]) pc += 2;
}

void Chip8::OP_ANNN()
{
	uint16_t address = current_opcode & 0x0fff;
	index = address;											//index = NNN
}

void Chip8::OP_BNNN()
{
	uint16_t address = current_opcode & 0x0fff;
	pc = address + registers[0];
}


//for OP_CXNN
std::uniform_int_distribution<int> distribution{ 0,255 };		    //0 to 255 covers all 8 bit numbers
std::default_random_engine generator{ std::random_device{}() };	//pseudo_random generator with random seed value (seeding happens at the very first call, after that prng is called)
void Chip8::OP_CXNN()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t nn = (current_opcode & 0x00ff);

	registers[vx] = nn & distribution(generator);
}

void Chip8::OP_DXYN()
{

	//coordinate x,y of (left) corner bit of sprite
	uint8_t x = registers[(current_opcode & 0x0f00) >> 8] % columns;			   //X
	uint8_t y = registers[(current_opcode & 0x00f0) >> 4] % rows;				   //Y
	uint8_t sprite_height = current_opcode & 0x000f;							   //N
	

	registers[0xF] = 0;															   //set VF = 0 if no pixels have been turned off (start with this assumption)

	for (uint8_t row_offset = 0; row_offset < sprite_height; row_offset++)
	{
		uint8_t row = y + row_offset;
		if (row == rows) break;										   //clipping vertically

		uint64_t display_row = display[row];
		uint16_t sprite_byte = index + row_offset;
		uint64_t sprite_row = (static_cast<uint64_t>(memory[sprite_byte]) << 56) >> x;   //fetch a byte from the sprite, and place the sprite on 64 bit long row

		if (display_row & sprite_row) registers[0xF] = 1;	                       //set VF = 1 if display pixel has been turned off  
		display[row] = display_row ^ sprite_row;
	}
}

void Chip8::OP_EXA1()
{
	//update_keystates();

	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t key_requested = registers[vx] & 0xF;  //the lower-four bits represent the acceptable values (0 to 15). Masking them alone ensures we do not get anything outside this range.
	

	if (keystates[key_requested] == false) pc += 2;

}

void Chip8::OP_EX9E()
{
	//update_keystates();

	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t key_requested = registers[vx] & 0xF; //the lower-four bits represent the acceptable values (0 to 15). Masking them alone ensures we do not get anything outside this range.
	

	if (keystates[key_requested] == true) pc += 2;
}


void Chip8::OP_FX07()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	registers[vx] = delay_timer;
}


void Chip8::OP_FX0A()
{
	//update_keystates();

	uint8_t vx = (current_opcode & 0x0F00) >> 8;

	for (const auto& key_pressed : keystates)
	{
		if (key_pressed == true)
		{
			registers[vx] = key_pressed;
			return;
		}
	}

	pc -= 2; // Repeat the instruction if no valid key is pressed
	
}

void Chip8::OP_FX15()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	delay_timer = registers[vx];
}

void Chip8::OP_FX18()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	sound_timer = registers[vx];
}

void Chip8::OP_FX1E()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;

	//if (index > 0 && registers[vx] > std::numeric_limits<uint32_t>::max() - index) //if index + VX overflows
	//{
	//	registers[0xF] = 1;
	//	/*
	//	Unlike other arithmetic instructions, this did not affect VF on overflow on the original COSMAC VIP. 
	//	However, it seems that some interpreters set VF to 1 if I “overflows” from 0FFF to above 1000 (outside the normal addressing range). 
	//	This wasn’t the case on the original COSMAC VIP, at least, but apparently the CHIP-8 interpreter for Amiga behaved this way. 
	//	At least one known game, Spacefight 2091!, relies on this behavior. 
	//	I don’t know of any games that rely on this not happening, so perhaps it’s safe to do it like the Amiga interpreter did.
	//	*/
	//}

	index += registers[vx];
}

void Chip8::OP_FX29()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t font_character = registers[vx];
	uint8_t font_start = 0x50;				//fonts are present starting from memory location 0x50, each occupying 5 bytes.
	index = font_start + (font_width * font_character);
}

void Chip8::OP_FX33()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;
	uint8_t number = registers[vx];

	for (int i = 2; i >= 0; i--)
	{
		if (index + i < 0 || index + i > 4095) continue; //skip any out of bound write attempts
		memory[index + i] = number % 10;
		number /= 10;
	}
}

void Chip8::OP_FX55()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;

	for (uint8_t i = 0; i <= vx; i++)
	{
		if (index < 0 || index > 4095) continue; //skip any out of bound write attempts
		memory[index++] = registers[i];
	}
}

void Chip8::OP_FX65()
{
	uint8_t vx = (current_opcode & 0x0f00) >> 8;

	for (uint8_t i = 0; i <= vx; i++)
	{
		if (index < 0 || index > 4095) continue; //skip any out of bound write attempts
		registers[i] = memory[index++];
	}
}

void Chip8::load_rom(const char* rom_path)
{
	std::ifstream rom(rom_path, std::ios::binary | std::ios::ate); //open file in binary format, and place the file pointer at end of file

	if (rom.is_open())
	{

		std::streampos size{ rom.tellg() };				    //tellg returns the "distance" of file pointer's current position (which is currently at the end) from start in bytes
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

void Chip8::update_keystates()
{
	for (const auto& [key, raykey] : key_to_raykey)
	{
		if (IsKeyDown(raykey))
		{
			keystates[key] = 1;
		}
		else if (IsKeyUp(raykey))
		{
			keystates[key] = 0;
		}
	}
}
