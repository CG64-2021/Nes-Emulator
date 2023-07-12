#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bus.h"
#include "ppu.h"
#include "cpu.h"
#include "tables.h"

ppu_t PPU_init(bus_t* bus)
{
	ppu_t ppu;
	memset(&ppu, 0, sizeof(ppu_t));
	ppu.bus = bus;
	ppu.PPU_addr.is_hi = 1;
	return ppu;
}

uint8_t PPU_read(ppu_t* ppu, uint16_t addr)
{
	return ppu->map[addr];
}

void PPU_write(ppu_t* ppu, uint8_t data, uint16_t addr)
{
	ppu->map[addr] = data;
}


void PPU_SetPPUADDR(ppu_t* ppu, uint16_t data)
{
	ppu->PPU_addr.hi = (data >> 8);
	ppu->PPU_addr.lo = (data & 0xFF);
}

uint16_t PPU_GetFullAddrFromPPUADDR(ppu_t* ppu)
{
	uint16_t data = (ppu->PPU_addr.hi << 8) | ppu->PPU_addr.lo;
	return data;
}

void PPU_IncrementPPUADDR(ppu_t* ppu, uint8_t inc)
{
	uint8_t lo = ppu->PPU_addr.lo;
	ppu->PPU_addr.lo += inc;
	if (lo > ppu->PPU_addr.lo) ppu->PPU_addr.hi++;
	
	if (PPU_GetFullAddrFromPPUADDR(ppu) > 0x3FFF) PPU_SetPPUADDR(ppu, PPU_GetFullAddrFromPPUADDR(ppu)&0x3FFF); //Mirror down above 0x3FFF
}

void PPU_ResetPPUADDRLatch(ppu_t* ppu)
{
	ppu->PPU_addr.is_hi = 1;
}


void PPU_UpdatePPUADDR(ppu_t* ppu, uint8_t data)
{
	if (ppu->PPU_addr.is_hi) ppu->PPU_addr.hi = data; else ppu->PPU_addr.lo = data;
	if (PPU_GetFullAddrFromPPUADDR(ppu) > 0x3FFF) PPU_SetPPUADDR(ppu, PPU_GetFullAddrFromPPUADDR(ppu)&0x3FFF);
	ppu->PPU_addr.is_hi = !ppu->PPU_addr.is_hi;
}

void PPU_WriteToPPUADDR(ppu_t* ppu, uint8_t data)
{
	PPU_UpdatePPUADDR(ppu, data);
}

uint8_t PPU_SetAddrIncrement(ppu_t* ppu)
{
	if (!ppu->PPU_ctrl&VRAM_ADD_INCREMENT) return 1; else return 32;
}

void PPU_UpdatePPUCTRL(ppu_t* ppu, uint8_t data)
{
	ppu->PPU_ctrl = data;
}

void PPU_WritetoPPUCTRL(ppu_t* ppu, uint8_t data)
{
	PPU_UpdatePPUCTRL(ppu, data);
}


uint16_t PPU_mirrorVRAMaddr(ppu_t* ppu, uint16_t addr)
{
	//Horizontal Mirroring
	if (!ppu->mirroring)
	{
		if (addr >= 0x2400 && addr <= 0x27FF) return addr-0x0400;
		if (addr >= 0x2C00 && addr <= 0x2FFF) return addr-0x0400;
		return addr;
	}
	//Vertical Mirroring
	if (addr >= 0x2800 && addr <= 0x2BFF) return addr-0x0800;
	if (addr >= 0x2C00 && addr <= 0x2FFF) return addr-0x0800;
	return addr;
}

uint8_t PPU_readDataFromPPUADDR(ppu_t* ppu)
{
	uint16_t addr = PPU_GetFullAddrFromPPUADDR(ppu); //Get addr from PPUADDR
	PPU_IncrementPPUADDR(ppu, PPU_SetAddrIncrement(ppu)); //Increment PPUADDR
	
	if (addr > 0x2FFF) return PPU_read(ppu, addr);
	
	//Because read/write from/to CHR-ROM and Nametables are slower than palettes,
	//we need to return a dummy data which contains the previous information to emulate the same
	//behavior as the original hardware
	if (addr < 0x2000)
	{
		//uint8_t result = ppu->internal_buffer;
		//ppu->internal_buffer = PPU_read(ppu, addr);
		//return result;
		return PPU_read(ppu, addr);
	}
	
	//uint8_t result = ppu->internal_buffer;
	//ppu->internal_buffer = PPU_read(ppu, PPU_mirrorVRAMaddr(ppu, addr));
	//return result;
	return PPU_read(ppu, addr);
}

void PPU_writeDataToPPUADDR(ppu_t* ppu)
{
	uint16_t addr = PPU_GetFullAddrFromPPUADDR(ppu);
	PPU_IncrementPPUADDR(ppu, PPU_SetAddrIncrement(ppu));
	
	if (addr > 0x2FFF)
	{
		PPU_write(ppu, ppu->bus->map[PPUDATA], addr);
		return;
	}
	
	if (addr < 0x2000)
	{
		//uint8_t data = ppu->internal_buffer;
		//ppu->internal_buffer = ppu->bus->map[PPUDATA];
		//PPU_write(ppu, data, addr);
		PPU_write(ppu, ppu->bus->map[PPUDATA], addr);
		return;
	}
	
	//uint8_t data = ppu->internal_buffer;
	//ppu->internal_buffer = ppu->bus->map[PPUDATA];
	//PPU_write(ppu, data, PPU_mirrorVRAMaddr(ppu, addr));
	PPU_write(ppu, ppu->bus->map[PPUDATA], addr);
}

void PPU_WriteToPPUSCROLL(ppu_t* ppu, uint8_t data)
{
	ppu->PPU_scroll.x = ppu->PPU_scroll.is_x? data : ppu->PPU_scroll.x;
	ppu->PPU_scroll.y = !ppu->PPU_scroll.is_x? data : ppu->PPU_scroll.y;
	ppu->PPU_scroll.is_x = !ppu->PPU_scroll.is_x;
}

uint8_t PPU_tick(ppu_t* ppu, uint16_t cycles)
{
	//Get clock from CPU and do some tasks
	ppu->clock += cycles;
	
	//Are we rendering next scanline?
	if (ppu->clock >= 341)
	{
		ppu->clock -= 341;
		ppu->scanline++;
	}
	//If we rendered 241 scanlines, enable NMI
	if (ppu->scanline == 241)
	{
		ppu->bus->ppu_register = PPUCTRL;
		ppu->bus->ppu_RW = 1;
		ppu->bus->map[PPUCTRL] |= GENERATE_NMI;
		ppu->PPU_status |= VBLANK;
	}
	//If we rendered all scanlines, reset
	if (ppu->scanline >= 262)
	{
		ppu->scanline = 0;
		ppu->PPU_status &= ~VBLANK;
		return 1;
	}
	
	return 0;
}

void PPU_setPixel(ppu_t* ppu, int x, int y, color_t color)
{
	int base = y*3*SCREEN_WIDTH+x*3;
	int len = SCREEN_WIDTH*SCREEN_HEIGHT*3;
	if ((base + 2) < len)
	{
		ppu->frame_buffer[base] = color.r;
		ppu->frame_buffer[base+1] = color.g;
		ppu->frame_buffer[base+2] = color.b;
	}
}

void PPU_renderTile(ppu_t* ppu, uint16_t bank, uint16_t tile_n)
{
	bank *= 0x1000;
	uint8_t* tile = &ppu->map[bank + tile_n * 16];
	
	for(int y=0; y < 8; ++y)
	{
		uint8_t upper = tile[y];
		uint8_t lower = tile[y+8];
		
		for(int x=0; x < 8; ++x)
		{
			uint8_t value = (1&upper) << 1 | (1&lower);
			upper = upper >> 1;
			lower = lower >> 1;
			
			color_t color;
			switch(value)
			{
				case 0: color = colors[0x01]; break;
				case 1: color = colors[0x23]; break;
				case 2: color = colors[0x27]; break;
				case 3: color = colors[0x30]; break;
			}
			PPU_setPixel(ppu, x, y, color);
		}
	}
}

void PPU_render(ppu_t* ppu)
{
	uint8_t bank = ppu->PPU_ctrl&BACKGROUND_PATTERN_ADDR;
	bank *= 0x1000;
	
	for(int i=0x0; i < 0x03C0; ++i)
	{
		uint8_t tile = ppu->map[i];
		uint16_t tile_x = i % 32;
		uint16_t tile_y = i / 32;
		uint8_t* tile_ptr = &ppu->map[(bank+tile*16)];
		
		for(int y=0; y < 8; ++y)
		{
			uint8_t upper = tile_ptr[y];
			uint8_t lower = tile_ptr[y+8];
			
			for(int x=0; x < 8; ++x)
			{
				uint8_t value = (1&upper) << 1 | (1&lower);
				upper = upper >> 1;
				lower = lower >> 1;
				color_t color;
				switch(value)
				{
					case 0: color = colors[0x01]; break;
					case 1: color = colors[0x23]; break;
					case 2: color = colors[0x27]; break;
					case 3: color = colors[0x30]; break;
				}
				PPU_setPixel(ppu, tile_x*8+x, tile_y*8+y, color);
			}
		}
	}
}

void PPU_update(ppu_t* ppu)
{
	//Check if we can update PPU
	if (PPU_tick(ppu, ppu->bus->cpu_clock*3)) return;
	
	//Update PPUCTRL register (Write only)
	if (ppu->bus->ppu_register == PPUCTRL && ppu->bus->ppu_RW)
		PPU_WritetoPPUCTRL(ppu, ppu->bus->map[PPUCTRL]);
	//Update PPUMASK register (Write only)
	if (ppu->bus->ppu_register == PPUMASK && ppu->bus->ppu_RW)
		ppu->PPU_mask = ppu->bus->map[PPUMASK];
	//Update PPUSTATUS register (Read only)
	ppu->bus->map[PPUSTATUS] = ppu->PPU_status;
	//Update OAMADDR register (Write only)
	if (ppu->bus->ppu_register == OAMADDR && ppu->bus->ppu_RW)
		ppu->PPU_OAMaddr = ppu->bus->map[OAMADDR];
	//Update OAMDATA register (R/W)
	if (ppu->bus->ppu_register == OAMDATA)
	{
		if (!ppu->bus->ppu_RW) ppu->bus->map[OAMDATA] = ppu->PPU_OAMdata;
		else ppu->PPU_OAMdata = ppu->bus->map[OAMDATA];
	}
	//Update PPUSCROLL register (Write only x2)
	if (ppu->bus->ppu_register == PPUSCROLL && ppu->bus->ppu_RW)
		PPU_WriteToPPUSCROLL(ppu, ppu->bus->map[PPUSCROLL]);
	//Update PPUADDR register //(Write only x2)
	if (ppu->bus->ppu_register == PPUADDR && ppu->bus->ppu_RW)
		PPU_WriteToPPUADDR(ppu, ppu->bus->map[PPUADDR]);
	//Update PPUDATA register //(R/W)
	if (ppu->bus->ppu_register == PPUDATA)
	{
		if (!ppu->bus->ppu_RW) 
			ppu->bus->map[PPUDATA] = PPU_readDataFromPPUADDR(ppu);
		else PPU_writeDataToPPUADDR(ppu);
	}
	//Update OAMDMA register (Write only)
	if (ppu->bus->ppu_register == OAMDATA && ppu->bus->ppu_RW)
	{
		for (int i=0; i < 0x100; ++i)
		{
			uint16_t addr = (ppu->bus->map[OAMDMA] << 8) | i;
			ppu->OAM[i] = ppu->bus->map[addr];
		}
	}
	
	PPU_render(ppu);
}