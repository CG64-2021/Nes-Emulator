#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL.h>

#include "rom.h"
#include "bus.h"
#include "ppu.h"
#include "cpu.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* surface = NULL;
SDL_Texture* texture = NULL;

void init()
{	
	//Init SDL2
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("\nError in init SDL2: %s\n", SDL_GetError());
		system("pause");
		exit(-1);
	}
	
	//Open window and init renderer
	window = SDL_CreateWindow("Nes Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	
	//Create texture to be updated all time
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	
}

uint8_t input()
{
	SDL_Event event;
	
	while(SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			return 1;
		}
		
		//Key pressing
		if (event.type == SDL_KEYDOWN)
		{
			//TODO
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	iNES_rom_t* rom;
	bus_t general_bus;
	cpu_t cpu;
	ppu_t ppu;
	char name[256];
	
	printf("Where's the ROM: ");
	scanf("%256[^\n]", name);
	printf("name: %s\n", name);
	
	//Load ROM to be emulated into memory
	rom = GP_LoadiNESROM(name);
	if (rom == NULL)
	{
		system("pause");
		return 0;
	}
	
	//Prinf ROM info
	printf("ID: %s\n", rom->header.id);
	printf("PRG size: %i\n", rom->header.PRG_size*KB16);
	printf("CHR size: %i\n", rom->header.CHR_size*KB8);
	printf("flags 6: 0x%X\n", rom->header.flags[0]);
	printf("flags 7: 0x%X\n", rom->header.flags[1]);
	printf("flags 8: 0x%X\n", rom->header.flags[2]);
	printf("flags 9: 0x%X\n", rom->header.flags[3]);
	printf("flags 10: 0x%X\n\n", rom->header.flags[4]);
	
	//Init SDL2
	init();
	
	//Init BUS
	general_bus = BUS_init();
	//Init CPU
	cpu = CPU_init(&general_bus);
	//Init PPU
	ppu = PPU_init(&general_bus);
	ppu.mirroring = rom->header.flags[0]&0x1; //Set mirroring to PPU (0 - horizontal, 1 - vertical)
	cpu.ppu = &ppu; //Points to PPU to do some tasks later
	
	//Write ROM contents to the BUS
	if (!(rom->header.flags[0] & 0xF0) && !(rom->header.flags[1] & 0xF0))
	{
		int i=0;
		uint16_t addr = 0x8000;
		uint16_t ppu_addr = 0x0;
		
		//Write PRG code from NROM 128
		for(i=0; rom->header.PRG_size == 1 && i < (rom->header.PRG_size*KB16*2); ++i)
		{
			mem_write(cpu.bus, rom->PRG_data[i%KB16], addr);
			addr++;
		}
		//Write PRG code from NROM 256
		for(i=0; rom->header.PRG_size > 1 && i < (rom->header.PRG_size*KB16); ++i)
		{
			mem_write(cpu.bus, rom->PRG_data[i], addr);
			addr++;
		}
		
		//Write CHR grafics to Pattern Tables
		for(i=0; rom->header.CHR_size >= 1 && i < (rom->header.CHR_size*KB8); ++i)
		{
			PPU_write(&ppu, rom->CHR_data[i], ppu_addr);
			ppu_addr++;
		}
	}
	else
	{
		printf("Sorry, but ROM mappers are not supported yet.\n");
		system("pause");
		return 0;
	}
	
	//After writing, reset CPU
	CPU_reset(&cpu);

	printf("register_A: 0x%X\nregister_X: 0x%X\nregister_Y: 0x%X\n\n", cpu.register_a, cpu.register_x, cpu.register_y);
	printf("Program counter: 0x%X\nStack Pointer: 0x%X\nStatus: 0x%X\n\n", cpu.program_counter, cpu.stack_pointer, cpu.status);
	
	while(1)
	{
		CPU_read(&cpu); //CPU's processing...
		PPU_update(&ppu); //PPU too...
		
		//Render tiles on screen
		//PPU_renderTile(&ppu, 0, 0);
		
		//printf("A:%X X:%X Y:%X PC:%X STK:%X STT:%X OP:%s ADDRM:%i\n", cpu.register_a, cpu.register_x, cpu.register_y, cpu.program_counter, cpu.stack_pointer, cpu.status, cpu.opcode.name, cpu.opcode.addr_mode);
		//printf("PC:%X OP:%X INST:%s OP+1:%X\n", cpu.program_counter-1, cpu.bus->map[cpu.program_counter-1], cpu.opcode.name, cpu.bus->map[cpu.program_counter]);
		//printf("Nametable 01: %X%X%X%X\n", ppu.map[0x2C00], ppu.map[0x2C01], ppu.map[0x2C02], ppu.map[0x2C03]);
		if (input()) break;
		
		SDL_UpdateTexture(texture, NULL, ppu.frame_buffer, 256*3);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Delay(NES_CPUCLOCK/60);
	}
	
	//Unload ROM
	GP_UnloadiNESROM(rom);
	
	return 0;
}