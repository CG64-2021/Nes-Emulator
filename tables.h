//File written by CG64
//You can use it for whatever purposes
//Brief: Lookup tables and instructions list

extern color_t colors[64];
extern opcode_t opcode_lookup_table[256];

//Instructions
extern void BRK(cpu_t* cpu);
extern void LDA(cpu_t* cpu, uint16_t value);
extern void LDX(cpu_t* cpu, uint16_t value);
extern void LDY(cpu_t* cpu, uint16_t value);
extern void STA(cpu_t* cpu, uint16_t value);
extern void STX(cpu_t* cpu, uint16_t value);
extern void STY(cpu_t* cpu, uint16_t value);
extern void TAX(cpu_t* cpu);
extern void TAY(cpu_t* cpu);
extern void TSX(cpu_t* cpu);
extern void TXA(cpu_t* cpu);
extern void TYA(cpu_t* cpu);
extern void TXS(cpu_t* cpu);
extern void ADC(cpu_t* cpu, uint16_t value);
extern void SBC(cpu_t* cpu, uint16_t value);
extern void ASL(cpu_t* cpu, uint16_t value);
extern void LSR(cpu_t* cpu, uint16_t value);
extern void AND(cpu_t* cpu, uint16_t value);
extern void ORA(cpu_t* cpu, uint16_t value);
extern void EOR(cpu_t* cpu, uint16_t value);
extern void PHA(cpu_t* cpu);
extern void PHP(cpu_t* cpu);
extern void PLA(cpu_t* cpu);
extern void PLP(cpu_t* cpu);
extern void ROL(cpu_t* cpu, uint16_t value);
extern void ROR(cpu_t* cpu, uint16_t value);
extern void BIT(cpu_t* cpu, uint16_t value);
extern void BMI(cpu_t* cpu, uint16_t value);
extern void BCC(cpu_t* cpu, uint16_t value);
extern void BCS(cpu_t* cpu, uint16_t value);
extern void BEQ(cpu_t* cpu, uint16_t value);
extern void BNE(cpu_t* cpu, uint16_t value);
extern void BPL(cpu_t* cpu, uint16_t value);
extern void BVC(cpu_t* cpu, uint16_t value);
extern void BVS(cpu_t* cpu, uint16_t value);
extern void CLC(cpu_t* cpu);
extern void CLD(cpu_t* cpu);
extern void CLI(cpu_t* cpu);
extern void CLV(cpu_t* cpu);
extern void SEC(cpu_t* cpu);
extern void SED(cpu_t* cpu);
extern void SEI(cpu_t* cpu);
extern void CMP(cpu_t* cpu, uint16_t value);
extern void CPX(cpu_t* cpu, uint16_t value);
extern void CPY(cpu_t* cpu, uint16_t value);
extern void DEC(cpu_t* cpu, uint16_t value);
extern void DEX(cpu_t* cpu);
extern void DEY(cpu_t* cpu);
extern void INC(cpu_t* cpu, uint16_t value);
extern void INX(cpu_t* cpu);
extern void INY(cpu_t* cpu);
extern void JMP(cpu_t* cpu, uint16_t value);
extern void JSR(cpu_t* cpu, uint16_t value);
extern void RTS(cpu_t* cpu);
extern void RTI(cpu_t* cpu);