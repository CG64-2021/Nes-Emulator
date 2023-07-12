#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "rom.h"

//Load iNES-format ROM
iNES_rom_t* GP_LoadiNESROM(char* filename)
{
	FILE* fp;
	iNES_rom_t* rom;
	
	//Open iNES file
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		printf("GP_LoadiNESROM: file pointer returned NULL!\n");
		return NULL;
	}
	
	//malloc rom before we get info
	rom = (iNES_rom_t*)malloc(sizeof(iNES_rom_t));
	if (rom == NULL)
	{
		printf("GP_LoadiNESROM: could not malloc rom!\n");
		fclose(fp);
		return NULL;
	}
	memset(rom, 0, sizeof(iNES_rom_t));
	
	//Read ROM header
	fread(&rom->header, sizeof(iNES_header_t), 1, fp);
	
	//Check if the file has valid id ('N','E','S', 0x1A)
	if (rom->header.id[0] != 'N' && rom->header.id[1] != 'E' && rom->header.id[2] != 'S' && rom->header.id[3] != 0x1A)
	{
		printf("GP_LoadiNESROM: invalid iNES file!\n");
		free(rom);
		fclose(fp);
		return NULL;
	}
	
	//malloc trainer_data (if present), PRG_data, CHR_data (if present), INSTROM_data (if present) and PROM_data (if present)
	if (rom->header.flags[0] & 0x4) rom->trainer_data = (uint8_t*)malloc(sizeof(uint8_t)*512); //512 bytes = 1KB/2
	rom->PRG_data = (uint8_t*)malloc(sizeof(uint8_t)*(rom->header.PRG_size*KB16)); //KB16 bytes = 16KB
	if (rom->header.CHR_size > 0) rom->CHR_data = (uint8_t*)malloc(sizeof(uint8_t)*(rom->header.CHR_size*KB8)); //KB8 bytes = 8KB
	if (rom->header.flags[1] & 0x2) rom->INSTROM_data = (uint8_t*)malloc(sizeof(uint8_t)*KB8);
	if (rom->header.flags[1] & 0x2) rom->PROM_data = (uint8_t*)malloc(sizeof(uint8_t)*32);
	
	if (rom->PRG_data == NULL) printf("Could not malloc PRG_ROM!\n");

	//read trainer_data (if present), PRG_data, CHR_data (if present), INSTROM_data (if present) and PROM_data (if present) from the file
	if (rom->header.flags[0] & 0x4) fread(rom->trainer_data, sizeof(uint8_t), 512, fp);
	fread(rom->PRG_data, sizeof(uint8_t), rom->header.PRG_size*KB16, fp);
	if (rom->header.CHR_size > 0) fread(rom->CHR_data, sizeof(uint8_t), rom->header.CHR_size*KB8, fp);
	if (rom->header.flags[1] & 0x2) fread(rom->INSTROM_data, sizeof(uint8_t), KB8, fp);
	if (rom->header.flags[1] & 0x2) fread(rom->PROM_data, sizeof(uint8_t), 32, fp);
	
	fclose(fp);
	return rom;
}

//Unload iNES-format ROM
void GP_UnloadiNESROM(iNES_rom_t* rom)
{
	if (rom->trainer_data) free(rom->trainer_data);
	if (rom->PRG_data) free(rom->PRG_data);
	if (rom->CHR_data) free(rom->CHR_data);
	if (rom->INSTROM_data) free(rom->INSTROM_data);
	if (rom->PROM_data) free(rom->PROM_data);
	free(rom);
}