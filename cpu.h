//File written by CG64 (2023)
//You can use it for whatever purposes
//Brief: CPU

//NES CPU clock in milliseconds
#define NES_CPUCLOCK 1789.773

//Status masks
#define C (1 << 0) //Carry bit
#define Z (1 << 1) //Zero
#define I (1 << 2) //Disable interrupts
#define D (1 << 3) //Decimal Mode (Unused)
#define B (1 << 4) //Break
#define U (1 << 5) //Unused
#define V (1 << 6) //Overflow
#define N (1 << 7) //Negative

typedef enum
{
	IMP, //Implicit
	ZP0, //Zero Page
	ZPY, //Zero Page, Y indexed
	ABS, //Absolute
	ABY, //Absolute, Y indexed
	IZX, //Indirect, X indexed
	IMM, //Immediate
	ZPX, //Zero Page, X indexed
	REL, //Relative
	ABX, //Absolute, X indexed
	IND, //Indirect
	IZY, //Indirect, Y indexed
	ACC  //Accumulator
} addr_mode_t;

typedef struct
{
	char name[3]; //name
	addr_mode_t addr_mode; //Addressing mode
	uint8_t clock; //How many clocks this opcode takes
} opcode_t;

typedef struct
{
	uint8_t register_a; //Accumulator
	uint8_t register_x;
	uint8_t register_y;
	
	uint16_t program_counter;
	uint8_t stack_pointer;
	uint8_t status;
	
	//CPU clock
	//If its value is greater than 0, don't execute more opcodes for while
	uint16_t clock;
	
	//Opcode being executed
	opcode_t opcode;
	
	//bus pointer (CPU is connected to the bus)
	bus_t* bus;
	//PPU pointer (To update PPU registers if necessary)
	ppu_t* ppu;
} cpu_t;

//Get an absolute address from the opcode location
//Return: 16-bit absolute address
uint16_t CPU_GetAbsAddr(cpu_t* cpu);

void CPU_reset(cpu_t* cpu); //Do some changes in CPU when reset request
cpu_t CPU_init(bus_t* bus); //Set initial values to CPU after its creation
void CPU_irq(cpu_t* cpu); //Do some changes in CPU when interrupt request
void CPU_nmi(cpu_t* cpu); //Do some changes in CPU when non-maskable request
void CPU_read(cpu_t* cpu); //Begin to read opcode if CPU clock is 0, otherwise decrement clock value

//Instructions
