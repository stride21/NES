#pragma once
#include <array>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <cstdint>
#include <iterator>
#include <vector>
class c6502
{
public:
	std::array<uint16_t, 0xFFFF> memory; //stack counts down 0x100 - 0x1FF; zero page 0-0xFF 
	uint8_t register_a = 0; // G1 bits 01  ALU
	uint8_t register_x = 0; // G2 bits 10
	uint8_t register_y = 0;// G3 bits 00
	uint8_t register_p; // only 6 bits are used. status register used to flag 
	uint16_t program_counter = 0;
	uint8_t stack_pointer = 0x200;// stack pointer $200-$1FF
	uint8_t  opcode = 0x00;   // Is the instruction byte
	uint8_t  cycles = 0;	   // Counts how many cycles the instruction has remaining
	uint32_t clock_count = 0;
	uint16_t value = 0;
	bool IMP_flag = false;


private:
	void LoadROM(const char* filename);
	void Reset();
	void NMI();
	void IRQ();
	// addressing modes

	void IMP();	void IMM();
	void ZP0();	void ZPX();
	void ZPY();	void REL();
	void ABS();	void ABX();
	void ABY();	void IND();
	void IZX();	void IZY();

	// Opcodes ======================================================
	// There are 56 "legitimate" opcodes provided by the 6502 CPU. I
	// have not modelled "unofficial" opcodes. As each opcode is 
	// defined by 1 byte, there are potentially 256 possible codes.
	// Codes are not used in a "switch case" style on a processor,
	// instead they are repsonisble for switching individual parts of
	// CPU circuits on and off. The opcodes listed here are official, 
	// meaning that the functionality of the chip when provided with
	// these codes is as the developers intended it to be. Unofficial
	// codes will of course also influence the CPU circuitry in 
	// interesting ways, and can be exploited to gain additional
	// functionality!
	//
	// These functions return 0 normally, but some are capable of
	// requiring more clock cycles when executed under certain
	// conditions combined with certain addressing modes. If that is 
	// the case, they return 1.
	//
	// I have included detailed explanations of each function in 
	// the class implementation file. Note they are listed in
	// alphabetical order here for ease of finding.

	void ADC();	void AND();	void ASL();	void BCC();
	void BCS();	void BEQ();	void BIT();	void BMI();
	void BNE();	void BPL(); void BRK();	void BVC();
	void BVS();	void CLC();	void CLD();	void CLI();
	void CLV();	void CMP();	void CPX();	void CPY();
	void DEC();	void DEX();	void DEY();	void EOR();
	void INC();	void INX();	void INY();	void JMP();
	void JSR();	void LDA();	void LDX();	void LDY();
	void LSR();	void NOP();	void ORA();	void PHA();
	void PHP();	void PLA();	void PLP();	void ROL();
	void ROR();	void RTI();	void RTS();	void SBC();
	void SEC();	void SED();	void SEI();	void STA();
	void STX();	void STY();	void TAX();	void TAY();
	void TSX();	void TXA();	void TXS();	void TYA();

};

