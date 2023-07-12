//File written by CG64 (2023)
//You can use it for whatever purposes
//Brief: BUS

typedef struct
{
	uint16_t ppu_register; //one of PPU registers (0x2000-0x2007)
	uint8_t ppu_RW; //0 - Read, 1 - Write to PPU only
	uint16_t cpu_clock; //Get clock cycles from CPU
	uint8_t map[0x10000]; //Memory map
} bus_t;

bus_t BUS_init();
uint8_t mem_read(bus_t* bus, uint16_t addr);
void mem_write(bus_t* bus, uint8_t data, uint16_t addr);
uint16_t mem_read_u16(bus_t* bus, uint16_t addr);
void mem_write_u16(bus_t* bus, uint16_t data, uint16_t addr);