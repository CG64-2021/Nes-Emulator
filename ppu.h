//File written by CG64 (2023)
//You can use it for whatever purposes
//Brief: PPU (Picture Processing Unit)
//Reference: https://www.nesdev.org/wiki/PPU

//Screen coordinates
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

//PPU registers
#define PPUCTRL   0x2000
#define PPUMASK   0x2001
#define PPUSTATUS 0x2002
#define OAMADDR   0x2003
#define OAMDATA   0x2004
#define PPUSCROLL 0x2005
#define PPUADDR   0x2006
#define PPUDATA   0x2007
#define OAMDMA    0x4014

//Bits from PPUCTRL
#define NAMETABLE1             0b00000001
#define NAMETABLE2             0b00000010
#define VRAM_ADD_INCREMENT     0b00000100
#define SPRITE_PATTERN_ADDR    0b00001000
#define BACKGROUND_PATTERN_ADDR 0b00010000
#define SPRITE_SIZE            0b00100000
#define MASTER_SLAVE_SELECT    0b01000000
#define GENERATE_NMI           0b10000000

//Bits from PPUSTATUS
#define SPRITE_OVERFLOW 0b00100000
#define SPRITE0_HIT     0b01000000
#define VBLANK          0b10000000

//Color struct to be used to create a lookup table
typedef struct
{
	uint8_t r; //Red
	uint8_t g; //Green
	uint8_t b; //Blue
} color_t;

//PPUADDR register
typedef struct
{
	uint8_t hi; //High byte of address
	uint8_t lo; //Low byte of address
	uint8_t is_hi; //Check if the value received is the high byte or not
} PPU_address_t;

//PPUSCROLL register
typedef struct
{
	uint8_t x; //X scroll
	uint8_t y; //Y scroll
	uint8_t is_x; //Check if value received is for X or Y scroll
} PPU_scroll_t;

typedef struct
{
	bus_t* bus; //Points to CPU bus to read PPU registers
	
	//Screen, literally
	uint8_t frame_buffer[SCREEN_WIDTH*SCREEN_HEIGHT*3];
	
	uint8_t PPU_ctrl; //Get info from 0x2000 (PPUCTRL)
	uint8_t PPU_mask; //Get info from 0x2001 (PPUMASK)
	uint8_t PPU_status; //Set info to 0x2002 (PPUSTATUS)
	uint8_t PPU_OAMaddr; //Get info from 0x2003 (OAMADDR)
	uint8_t PPU_OAMdata; //Get info from 0x2004 (OAMDATA)
	PPU_scroll_t PPU_scroll; //Get info from 0x2005 (PPUSCROLL)
	PPU_address_t PPU_addr; //Get info from 0x2006 (PPUADDR) and put them to PPU_address_t register
	uint8_t internal_buffer; //Return internal_buffer when CPU's writing/reading to/from memory positions smaller than 0x3000 (Dummy data)
	uint8_t mirroring; //0 - horizontal, 1 - vertical
	
	//PPU Memory Map
	//(0x0000-0x0FFF) Pattern Table 0 (CHR-ROM)
	//(0x1000-0x1FFF) Pattern Table 1 (CHR-ROM)
	//(0x2000-0x23FF) Nametable 0 (VRAM)
	//(0x2400-0x27FF) Nametable 1 (VRAM)
	//(0x2800-0x2BFF) Nametable 2 (VRAM)
	//(0x2C00-0x2FFF) Nametable 3 (VRAM)
	//(0x3000-0x3EFF) Mirrors of 2000-2EFF
	//(0x3F00-0x3F1F) Pallete RAM indexes
	//(0x3F20-0x3FFF) Mirrors of 3F00-3F1F
	uint8_t map[0x4000];
	
	//OAM Memory Map
	//Byte 0 - Sprite Y Coordinate
	//Byte 1 - Sprite tile #
	//Byte 2 - Sprite Attribute
	//Byte 3 - Sprite X Coordinate
	uint8_t OAM[0x0100];
	
	//How many scanlines were rendered
	uint16_t scanline;
	
	//PPU clock works differently from CPU clock
	//It's 3 times faster than CPU clock, so if CPU clock is 1, PPU clock can run 3 clocks
	//If its cycles >= 341, cycles-=341 and increments 'scanline' variable
	//If its cycles == 241, trigger NMI
	//If its cycles >= 262, reset it and reset vblank status
	uint16_t clock;
} ppu_t;

ppu_t PPU_init(bus_t* bus); //Init PPU struct
uint8_t PPU_read(ppu_t* ppu, uint16_t addr); //Read a byte directly from PPU bus
void PPU_write(ppu_t* ppu, uint8_t data, uint16_t addr); //Write a byte directly to PPU bus

//Functions related to PPUADDR register
void PPU_SetPPUADDR(ppu_t* ppu, uint16_t data); //Set values to PPU_address_t from 16-bit data
void PPU_UpdatePPUADDR(ppu_t* ppu, uint8_t data); //Update PPU_address_t
void PPU_IncrementPPUADDR(ppu_t* ppu, uint8_t inc); //Increment low and high bytes of PPU_address_t after read/write from/to 0x2007
void PPU_ResetPPUADDRLatch(ppu_t* ppu); //Reset is_hi variable to 'true'
uint16_t PPU_GetFullAddrFromPPUADDR(ppu_t* ppu); //Return full address from PPU_address_t
void PPU_WriteToPPUADDR(ppu_t* ppu, uint8_t data); //Write to high/low byte of PPUADDR
//Functions related to PPUCTRL register
uint8_t PPU_SetAddrIncrement(ppu_t* ppu); //Return increment value to PPUADDR value+=inc
void PPU_UpdatePPUCTRL(ppu_t* ppu, uint8_t data); //Update PPUCTRL bits
void PPU_WritetoPPUCTRL(ppu_t* ppu, uint8_t data); //Write a byte to PPUCTRL
//Functions related to PPUSCROLL register
void PPU_WriteToPPUSCROLL(ppu_t* ppu, uint8_t data); //Update PPUSCROLL register

//More functions related to PPU
uint16_t PPU_mirrorVRAMaddr(ppu_t* ppu, uint16_t addr); //Return mirrored address based on Horizontal or Vertical mirroring in ROM
uint8_t PPU_readData(ppu_t* ppu); //Read data from PPU bus at address 'PPUADDR'
void PPU_writeData(ppu_t* ppu); //Write data from 'PPUDATA' to PPU bus at address 'PPUADDR'
uint8_t PPU_tick(ppu_t* ppu, uint16_t cycles); //Check if we can update PPU (0 - false, 1 - true)
void PPU_setPixel(ppu_t* ppu, int x, int y, color_t color); //Draw pixel on screen
void PPU_renderTile(ppu_t* ppu, uint16_t bank, uint16_t tile_n); //Draw tiles on screen
void PPU_render(ppu_t* ppu); //Draw background
void PPU_update(ppu_t* ppu); //Do general tasks in PPU