//File written by CG64 (2023)
//You can use it for whatever purposes
//Brief: Load/Unload NES ROMs to be emulated
//Reference: https://www.nesdev.org/wiki/INES

#define KB8 8192 //8KB - 8192 bytes
#define KB16 16384 //16KB - 16384 bytes

//iNES header
typedef struct
{
	char id[4]; //Constant $4E $45 $53 $1A (ASCII "NES" followed by MS-DOS end-of-file)
	uint8_t PRG_size; //Size of PRG ROM in 16 KB units
	uint8_t CHR_size; //Size of CHR ROM in 8 KB units (value 0 means the board uses CHR RAM)
	
	//Array positions:
	//0 - Mapper, mirroring, battery, trainer
	//1 - Mapper, VS/Playchoice, NES 2.0
	//2 - PRG-RAM size (rarely used extension)
	//3 - TV system (rarely used extension)
	//4 - TV system, PRG-RAM presence (unofficial, rarely used extension)
	uint8_t flags[5];
	
	uint8_t unused[5]; //Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)
} iNES_header_t;

//iNES rom
typedef struct
{
	iNES_header_t header;
	uint8_t* trainer_data; //Trainer data, if present (0 or 512 bytes)
	uint8_t* PRG_data; //PRG ROM data (16384 * x bytes)
	uint8_t* CHR_data; //CHR ROM data, if present (8192 * y bytes)
	uint8_t* INSTROM_data; //PlayChoice INST-ROM data, if present (0 or 8192 bytes)
	uint8_t* PROM_data; //PlayChoice PROM data, if present (16 bytes Data, 16 bytes CounterOut)
} iNES_rom_t;

iNES_rom_t* GP_LoadiNESROM(char* filename); //"GP" stands for "Game Pak"
void GP_UnloadiNESROM(iNES_rom_t* rom);