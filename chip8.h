#pragma once
#pragma once

#include<chrono>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <random>

#include "raylib.h"
//#include "chip8_clock.h"


extern Color off;
extern Color on;

class Chip8
{
private:

	uint8_t memory[4096]{};			//4 kB memory
	uint64_t display[32]{};			//32 elements, each 64 bit totalling to 64*32 bits/pixels
	uint16_t pc{};					//program counter
	uint16_t index{};					//index register
	uint16_t stack[16]{};				//16 level stack
	uint8_t sp{};						//stack pointer
	uint8_t registers[16]{};			//16 8 bit registers, V0 to VF
	uint8_t delay_timer{};			//delay timer
	uint8_t sound_timer{};			//sound timer
	uint16_t current_opcode{};

	//Clock clock;

	static inline const unsigned fontset_size = 16 * 5;
	uint8_t font[fontset_size] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0,// 0
		0x20, 0x60, 0x20, 0x20, 0x70,// 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0,// 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0,// 3
		0x90, 0x90, 0xF0, 0x10, 0x10,// 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0,// 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0,// 6
		0xF0, 0x10, 0x20, 0x40, 0x40,// 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0,// 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0,// 9
		0xF0, 0x90, 0xF0, 0x90, 0x90,// A
		0xE0, 0x90, 0xE0, 0x90, 0xE0,// B
		0xF0, 0x80, 0x80, 0x80, 0xF0,// C
		0xE0, 0x90, 0x90, 0x90, 0xE0,// D
		0xF0, 0x80, 0xF0, 0x80, 0xF0,// E
		0xF0, 0x80, 0xF0, 0x80, 0x80 // F
	};

	//for OP_CXNN
	static inline std::uniform_int_distribution<int> distribution{ 0,255 };		//0 to 255 covers all 8 bit numbers
	static inline std::default_random_engine generator{ std::random_device{}() };	//pseudo_random generator with random seed value (seeding happens at the very first call, after that prng is called)

public:
	Chip8();
	void load_rom(const char* rom_path);
	void increment_pc();
	uint16_t get_opcode();
	//Clock& getClock();

	void update_screen();
	static inline const size_t rows = 32;
	static inline const size_t columns = 64;
	static inline const size_t upscale_factor = 15;

	void OP_00E0();
	void OP_00EE();
	void OP_1NNN();
	void OP_2NNN();
	void OP_3XNN();
	void OP_4XNN();
	void OP_5XY0();
	void OP_6XNN();
	void OP_7XNN();
	void OP_8XY0();
	void OP_8XY1();
	void OP_8XY2();
	void OP_8XY3();
	void OP_8XY4();
	void OP_8XY5();
	void OP_8XY6();
	void OP_8XY7();
	void OP_8XYE();
	void OP_9XY0();
	void OP_ANNN();
	void OP_BNNN();
	void OP_CXNN();
	void OP_DXYN();
	void OP_FX1E();
	void OP_FX33();
	void OP_FX55();
	void OP_FX65();


};
