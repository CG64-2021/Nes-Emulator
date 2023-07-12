#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bus.h"
#include "ppu.h"
#include "cpu.h"

bus_t BUS_init()
{
	bus_t bus;
	memset(&bus, 0, sizeof(bus_t));
	return bus;
}

uint8_t mem_read(bus_t* bus, uint16_t addr)
{
	if (addr == PPUSTATUS || addr == PPUDATA)
	{
		//printf("Detected read on 0x%X.\n", addr);
		bus->ppu_register = addr;
		bus->ppu_RW = 0;
	}
	return bus->map[addr];
}

void mem_write(bus_t* bus, uint8_t data, uint16_t addr)
{
	if ((addr >= PPUCTRL && addr <= PPUADDR) || addr == OAMDMA)
	{
		//printf("Detected write on 0x%X.\n", addr);
		bus->ppu_register = addr;
		bus->ppu_RW = 1;
	}
	bus->map[addr] = data;
}

uint16_t mem_read_u16(bus_t* bus, uint16_t addr)
{
	uint16_t data;
	uint8_t lo = mem_read(bus, addr);
	uint8_t hi = mem_read(bus, addr+1);
	
	data = (hi << 8) | lo;
	return data;
}

void mem_write_u16(bus_t* bus, uint16_t data, uint16_t addr)
{
	uint8_t hi = (data >> 8);
	uint8_t lo = (data & 0xFF);
	
	//NES CPU is little-endian, so low byte must be written first
	mem_write(bus, lo, addr);
	mem_write(bus, hi, addr+1);
}