#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bus.h"
#include "ppu.h"
#include "cpu.h"
#include "tables.h"

void CPU_reset(cpu_t* cpu)
{
	cpu->program_counter = mem_read_u16(cpu->bus, 0xFFFC);
	cpu->register_a = 0;
	cpu->register_x = 0;
	cpu->register_y = 0;
	cpu->stack_pointer = 0xFD;
	cpu->status = 0x00 | U;
	
	//Reset takes time
	cpu->clock = 8;
}

cpu_t CPU_init(bus_t* bus)
{
	cpu_t cpu;
	cpu.bus = bus;
	CPU_reset(&cpu);
	
	return cpu;
}

void CPU_irq(cpu_t* cpu)
{
	uint8_t flag = (cpu->status & I);
	if (!flag)
	{
		//Push the program counter to the stack
		mem_write_u16(cpu->bus, cpu->program_counter, 0x0100+(cpu->stack_pointer-1));
		cpu->stack_pointer -= 2;
		
		//Push status register to the stack
		cpu->status &= ~B;
		cpu->status |= U;
		cpu->status |= I;
		mem_write(cpu->bus, cpu->status, 0x0100+cpu->stack_pointer);
		cpu->stack_pointer--;
		
		//Read new program_counter location from fixed address
		cpu->program_counter = mem_read_u16(cpu->bus, 0xFFFE);
		
		//IRQs take time
		cpu->clock = 7;
	}
}

void CPU_nmi(cpu_t* cpu)
{
	mem_write_u16(cpu->bus, cpu->program_counter, 0x0100+(cpu->stack_pointer-1));
	cpu->stack_pointer -= 2;
	
	cpu->program_counter = mem_read_u16(cpu->bus, 0xFFFA);
	
	cpu->clock = 8;
}

//Get an absolute address from the opcode location
//Return: 16-bit absolute address
uint16_t CPU_GetAbsAddr(cpu_t* cpu)
{
	opcode_t opcode = cpu->opcode;
	uint16_t data, aux;
	
	switch(opcode.addr_mode)
	{
		case ABS:
			data = mem_read_u16(cpu->bus, cpu->program_counter);
			cpu->program_counter+=2;
		break;
		case ABX:
			data = mem_read_u16(cpu->bus, cpu->program_counter);
			aux = (data&0xFF00);
			data += cpu->register_x;
			if ((data&0xFF00) != aux) cpu->clock++;
			cpu->program_counter+=2;
		break;
		case ABY:
			data = mem_read_u16(cpu->bus, cpu->program_counter);
			aux = (data&0xFF00);
			data += cpu->register_y;
			if ((data&0xFF00) != aux) cpu->clock++;
			cpu->program_counter+=2;
		break;
		case ZP0:
			data = (uint16_t)mem_read(cpu->bus, cpu->program_counter);
			cpu->program_counter++;
		break;
		case ZPX:
			data = (uint16_t)mem_read(cpu->bus, cpu->program_counter);
			data += cpu->register_x;
			data %= 256;
			cpu->program_counter++;
		break;
		case ZPY:
			data = (uint16_t)mem_read(cpu->bus, cpu->program_counter);
			data += cpu->register_y;
			data %= 256;
			cpu->program_counter++;
		break;
		case IZX:
			data = (uint16_t)mem_read(cpu->bus, cpu->program_counter);
			data += cpu->register_x;
			data %= 256;
			aux = mem_read_u16(cpu->bus, data);
			data = aux;
			cpu->program_counter++;
		break;
		case IZY:
			data = (uint16_t)mem_read(cpu->bus, cpu->program_counter);
			aux = mem_read_u16(cpu->bus, data);
			data = aux+cpu->register_y;
			aux = ((data-cpu->register_y)&0xFF00);
			if ((data&0xFF00) != aux) cpu->clock++;
			cpu->program_counter++;
		break;
	}
	
	return data;
}

void CPU_read(cpu_t* cpu)
{
	if (cpu->status&B) CPU_irq(cpu);
	if (mem_read(cpu->bus, PPUSTATUS)&GENERATE_NMI) CPU_nmi(cpu);
	
	if (!cpu->clock)
	{
		//Read opcode
		uint8_t data = mem_read(cpu->bus, cpu->program_counter);
		cpu->opcode = opcode_lookup_table[data];
		cpu->program_counter++;
		
		//Get starting number of cycles
		cpu->clock = opcode_lookup_table[data].clock;
		
		//Get absolute address
		uint16_t addr = 0;
		if (cpu->opcode.addr_mode != IMP && cpu->opcode.addr_mode != IMM && cpu->opcode.addr_mode != IND && cpu->opcode.addr_mode != ACC && 
			cpu->opcode.addr_mode != REL)
		{
			addr = CPU_GetAbsAddr(cpu);
			
			//Get CPU clock
			cpu->bus->cpu_clock = cpu->clock;
			
			//Update PPU if necessary
			//if (addr >= 0x2000 && addr <= 0x2007)
				//PPU_update(cpu->ppu);
		}
		
		//Execute instructions
		uint16_t value = addr > 0 ? addr : (uint16_t)mem_read(cpu->bus, cpu->program_counter);
		if (!memcmp(cpu->opcode.name, "BRK", 3)) {BRK(cpu); return;}
		if (!memcmp(cpu->opcode.name, "ADC", 3)) {ADC(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "SBC", 3)) {SBC(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "LDA", 3)) {LDA(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "LDX", 3)) {LDX(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "LDY", 3)) {LDY(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "STA", 3)) {STA(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "STX", 3)) {STX(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "STY", 3)) {STY(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "TAX", 3)) {TAX(cpu); return;}
		if (!memcmp(cpu->opcode.name, "TAY", 3)) {TAY(cpu); return;}
		if (!memcmp(cpu->opcode.name, "TSX", 3)) {TSX(cpu); return;}
		if (!memcmp(cpu->opcode.name, "TXA", 3)) {TXA(cpu); return;}
		if (!memcmp(cpu->opcode.name, "TYA", 3)) {TYA(cpu); return;}
		if (!memcmp(cpu->opcode.name, "TXS", 3)) {TXS(cpu); return;}
		if (!memcmp(cpu->opcode.name, "ASL", 3)) {ASL(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "LSR", 3)) {LSR(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "NOP", 3)) return;
		if (!memcmp(cpu->opcode.name, "AND", 3)) {AND(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "ORA", 3)) {ORA(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "EOR", 3)) {EOR(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "PHA", 3)) {PHA(cpu); return;}
		if (!memcmp(cpu->opcode.name, "PHP", 3)) {PHP(cpu); return;}
		if (!memcmp(cpu->opcode.name, "PLA", 3)) {PLA(cpu); return;}
		if (!memcmp(cpu->opcode.name, "PLP", 3)) {PLP(cpu); return;}
		if (!memcmp(cpu->opcode.name, "ROL", 3)) {ROL(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "ROR", 3)) {ROR(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BIT", 3)) {BIT(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BMI", 3)) {BMI(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BCC", 3)) {BCC(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BCS", 3)) {BCS(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BEQ", 3)) {BEQ(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BNE", 3)) {BNE(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BPL", 3)) {BPL(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BVC", 3)) {BVC(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "BVS", 3)) {BVS(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "CLC", 3)) {CLC(cpu); return;}
		if (!memcmp(cpu->opcode.name, "CLD", 3)) {CLD(cpu); return;}
		if (!memcmp(cpu->opcode.name, "CLI", 3)) {CLI(cpu); return;}
		if (!memcmp(cpu->opcode.name, "CLV", 3)) {CLV(cpu); return;}
		if (!memcmp(cpu->opcode.name, "SEC", 3)) {SEC(cpu); return;}
		if (!memcmp(cpu->opcode.name, "SED", 3)) {SED(cpu); return;}
		if (!memcmp(cpu->opcode.name, "SEI", 3)) {SEI(cpu); return;}
		if (!memcmp(cpu->opcode.name, "CMP", 3)) {CMP(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "CPX", 3)) {CPX(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "CPY", 3)) {CPY(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "DEC", 3)) {DEC(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "DEX", 3)) {DEX(cpu); return;}
		if (!memcmp(cpu->opcode.name, "DEY", 3)) {DEY(cpu); return;}
		if (!memcmp(cpu->opcode.name, "INC", 3)) {INC(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "INX", 3)) {INX(cpu); return;}
		if (!memcmp(cpu->opcode.name, "INY", 3)) {INY(cpu); return;}
		if (!memcmp(cpu->opcode.name, "JMP", 3)) {JMP(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "JSR", 3)) {JSR(cpu, value); return;}
		if (!memcmp(cpu->opcode.name, "RTS", 3)) {RTS(cpu); return;}
		if (!memcmp(cpu->opcode.name, "RTI", 3)) {RTI(cpu); return;}
		
		return;
	}
	cpu->clock--;
	return;
}