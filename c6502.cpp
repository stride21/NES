#include "c6502.h"

void c6502::LoadROM(const char* filename)
{
	const long pos = 0x4020;
	std::ifstream input(filename, std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
	for (unsigned long i = 0; i < buffer.size(); i++)
	{
		memory[pos + i] = buffer[i];
	}
}
//Addressing modes
void c6502::IMP()
{
	//do nothing
}

void c6502::IMM()
{
	value = memory[program_counter];
	++program_counter;
}

void c6502::ZP0()
{
	uint8_t temp = memory[program_counter];
	++program_counter;
	value = memory[temp];
}

void c6502::ZPX()
{
	uint8_t temp = memory[program_counter];
	++program_counter;
	value = ((temp + register_x) % 256);
}

void c6502::ZPY()
{
	uint8_t temp = memory[program_counter];
	++program_counter;
	value = ((temp + register_y) % 256);
}

void c6502::REL()
{
	value = memory[program_counter];
	++program_counter;
	if (value & 0x80)
	{
		value |= 0xFF00;
	}
}

void c6502::ABS()
{
	uint16_t low = memory[program_counter];
	++program_counter;
	uint16_t high = memory[program_counter];
	++program_counter;
	value = high << 8 | low;
}

void c6502::ABY()
{
	uint16_t low = memory[program_counter];
	++program_counter;
	uint16_t high = memory[program_counter];
	++program_counter;
	value = high << 8 | low;
	value += register_y;
	if ((value & 0xFF00) != (high << 8))
	{
		++cycles;
	}
}

void c6502::ABX()
{
	uint16_t low = memory[program_counter];
	++program_counter;
	uint16_t high = memory[program_counter];
	++program_counter;
	value = high << 8 | low;
	value += register_x;
	if ((value & 0xFF00) != (high << 8))
	{
		++cycles;
	}
}

void c6502::IND()
{
	uint16_t low = memory[program_counter];
	++program_counter;
	uint16_t high = memory[program_counter];
	++program_counter;
	uint16_t temp = high << 8 | low;
	if (low & 0xFF)
	{
		value = memory[temp & 0xFF00] << 8 | memory[temp + 0];
	}
	else
	{
		value = memory[temp + 1] << 8 | memory[temp + 0];
	}
}

void c6502::IZX()
{
	uint16_t low = memory[program_counter + register_x];
	++program_counter;
	uint16_t high = memory[program_counter + register_x];
	++program_counter;
	value = high << 8 | low;
}

void c6502::IZY()
{
	uint16_t low = memory[program_counter];
	++program_counter;
	uint16_t high = memory[program_counter];
	++program_counter;
	value = high << 8 | low;
	value += register_y;
}
void c6502::NMI()
{
	register_p |= 0x08;
	memory[0x100 + stack_pointer] = (program_counter >> 8) & 0x00FF;
	stack_pointer--;
	memory[0x100 + stack_pointer] = program_counter & 0x00FF;
	stack_pointer--;
	register_p ^= 0x10;
	register_p |= 0x08;
	memory[0x100 + stack_pointer] = register_p;
	stack_pointer--;
	program_counter = (uint16_t)memory[0xFFFA] | ((uint16_t)memory[0xFFFB] << 8);
}

void c6502::IRQ()
{ 
	register_p |= 0x08;
	memory[0x100 + stack_pointer] = (program_counter >> 8) & 0x00FF;
	stack_pointer--;
	memory[0x100 + stack_pointer] = program_counter & 0x00FF;
	stack_pointer--;
	register_p ^= 0x10;
	register_p |= 0x08;
	memory[0x100 + stack_pointer] = register_p;
	stack_pointer--;
//	register_p ^= 0x10;
	program_counter = (uint16_t)memory[0xFFFE] | ((uint16_t)memory[0xFFFF] << 8);
}
void c6502::Reset()
{
	program_counter = (uint16_t)memory[0xFFFC] | ((uint16_t)memory[0xFFFD] << 8);
}
// Opcodes
//BRK causes a non - maskable interrupt and increments the program counter by one.Therefore an RTI will go to the 
//address of the BRK + 2 so that BRK may be used to replace a two - byte instruction for debugging and the subsequent RTI will be correct.
void c6502::BRK()
{
	program_counter++;
	register_p |= 0x08;
	memory[0x100 + stack_pointer] = (program_counter >> 8) & 0x00FF;
	stack_pointer--;
	memory[0x100 + stack_pointer] = program_counter & 0x00FF;
	stack_pointer--;
	register_p |= 0x10;
	memory[0x100 + stack_pointer] = register_p;
	stack_pointer--;
	//register_p ^= 0x10;
	program_counter = (uint16_t)memory[0xFFFE] | ((uint16_t)memory[0xFFFF] << 8);

}

void c6502::BPL()
{
	//check if negative flag is unset
	if (register_p & 0x80 == 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}

void c6502::ORA()
{
	register_a = register_a | memory[value];
}

void c6502::ADC()
{
	uint16_t temp = (uint16_t)register_a + (uint16_t)memory[value] + (uint16_t)(register_p & 0x01);
	if (temp > 255)
	{
		register_p | 0x01;
	}
	if (temp == 0)
	{
		register_p | 0x02;
	}
	if (temp & 0x80 == 1)
	{
		register_p = register_p | 0x80;
	}
	if ((~((uint16_t)register_a ^ (uint16_t)memory[value]) & ((uint16_t)register_a ^ (uint16_t)temp)) & 0x0080)
	{
		register_p = register_p | 0x40;
	}
	register_a = temp & 0x00FF;
	++cycles;
}

void c6502::AND()
{
	register_a = register_a & memory[value];
}
//ASL shifts all bits left one position. 0 is shifted into bit 0 and the original bit 7 is shifted into the Carry.
void c6502::ASL()
{
	uint16_t temp = memory[value] << 1;
	uint8_t MSB = memory[value] >> 7;

	// add 7 bit to carry
	register_p |= MSB;
	// set negative flag
	register_p |= (temp & 0x80);
	// set zero flag 
	if (temp == 0)
	{
		register_p |= 0x40;
	}
	if (IMP_flag == true)
	{
		register_a = temp & 0x00FF;
	}
	else
	{
		memory[value] = temp & 0x00FF;
	}
}

void c6502::BCC()
{
	// check if carry flag is unset
	if (register_p & 0x01 == 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}

void c6502::BCS()
{
	// check if carry flag is set
	if (register_p & 0x01 != 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}

void c6502::BEQ()
{
	//check if zero flag is set
	if (register_p & 0x02 != 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}
//BIT sets the Z flag as though the value in the address tested were ANDed with the accumulator.
//The S and V flags are set to match bits 7 and 6 respectively in the value stored at the tested address.
void c6502::BIT()
{
	uint8_t temp = memory[value] & (register_a & 0x80); // AND Z bit with both memory and register_a
	if (temp == 0x00)
	{
		register_p |= 0x80;
	}
	register_p |= (memory[value] & 0x40);
	register_p |= (memory[value] & 0x20);
}

void c6502::BMI()
{
	//check if negative flag is set
	if (register_p & 0x80 != 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}

void c6502::BNE()
{
	//check if zero flag is unset
	if (register_p & 0x02 == 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}

void c6502::BVC()
{
	//check if overflow flag is unset
	if (register_p & 0x40 == 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}

void c6502::BVS()
{
	//check if overflow flag is set
	if (register_p & 0x40 != 0)
	{
		cycles++;
		uint16_t address = program_counter + value;
		// add cycle if page changed
		if ((address & 0xFF00) != (program_counter & 0xFF00))
		{
			cycles++;
		}
		program_counter = address;
	}
}

void c6502::CLD()
{
	register_p ^= 0x08;
}

void c6502::CLI()
{
	register_p ^= 0x04;
}

void c6502::CLV()
{
	register_p ^= 0x40;
}

void c6502::CPX()
{
	uint16_t temp = (uint16_t)register_a - (uint16_t)register_x;
	if (register_a >= memory[value])
	{
		register_a |= 0x01;
	}
	if ((temp & 0xFF) == 0)
	{
		register_a |= 0x40;
	}
	if (temp & 0x80 != 0)
	{
		register_a |= 0x80;
	}
}

void c6502::CPY()
{
	uint16_t temp = (uint16_t)register_a - (uint16_t)register_y;
	if (register_a >= memory[value])
	{
		register_a |= 0x01;
	}
	if ((temp & 0xFF) == 0)
	{
		register_a |= 0x40;
	}
	if (temp & 0x80 != 0)
	{
		register_a |= 0x80;
	}
}

void c6502::CLC()
{
	register_p ^= 0x01;
}

void c6502::DEC()
{
	--program_counter;
}

void c6502::EOR()
{
	register_a = register_a ^ memory[value];
}

void c6502::INC()
{
	program_counter++;
}

void c6502::JMP()
{
	program_counter = value;
}

void c6502::JSR()
{
	program_counter--;
	memory[0x100 + stack_pointer] = (program_counter >> 8) & 0x00FF;
	stack_pointer--;
	memory[0x100 + stack_pointer] = program_counter & 0x00FF;
	stack_pointer--;
	program_counter = value;
}

void c6502::LDA()
{
	register_a = memory[value];
}

void c6502::LDX()
{
	register_x = memory[value];
}

void c6502::LDY()
{
	register_y = memory[value];
}
//LSR shifts all bits right one position. 0 is shifted into bit 7 and the original bit 0 is shifted into the Carry.
void c6502::LSR()
{
	uint16_t temp = memory[value] >> 1;
	uint8_t LSB = memory[value] << 7;

	//add carry to value
	temp &= (register_p & 0x01);
	// add 0 bit to carry
	register_p |= LSB;
	// set negative flag
	register_p |= (temp & 0x80);
	// set zero flag 
	if (temp == 0)
	{
		register_p |= 0x40;
	}
	if (IMP_flag == true)
	{
		register_a = temp & 0x00FF;
	}
	else
	{
		memory[value] = temp & 0x00FF;
	}
}

void c6502::NOP()
{
	// does nothing
}

void c6502::PHA()
{
	memory[stack_pointer] = register_a;
	stack_pointer--;
}

void c6502::PHP()
{
	memory[stack_pointer] = register_p;
	stack_pointer--;
}

void c6502::PLA()
{
	register_a = memory[stack_pointer];
	stack_pointer++;
}

void c6502::PLP()
{
	register_p = memory[stack_pointer];
	stack_pointer++;
}
//ROL shifts all bits left one position.The Carry is shifted into bit 0 and the original bit 7 is shifted into the Carry.
void c6502::ROL()
{
	uint16_t temp = memory[value] >> 1;
	uint8_t LSB = memory[value] << 7;
	// add 0 bit to carry
	register_p |= LSB;
	// set negative flag
	register_p |= (temp & 0x80);
	// set zero flag 
	if (temp == 0)
	{
		register_p |= 0x40;
	}
	if (IMP_flag == true)
	{
		register_a = temp & 0x00FF;
	}
	else
	{
		memory[value] = temp & 0x00FF;
	}
	IMP_flag = false;
}

void c6502::ROR()
{
	uint16_t temp = memory[value] << 1;
	uint8_t MSB = memory[value] >> 7;

	//add carry to value
	temp &= (register_p & 0x01);
	// add 7 bit to carry
	register_p |= MSB;
	// set negative flag
	register_p |= (temp & 0x80);
	// set zero flag 
	if (temp == 0)
	{
		register_p |= 0x40;
	}
	if (IMP_flag == true)
	{
		register_a = temp & 0x00FF;
	}
	else
	{
		memory[value] = temp & 0x00FF;
	}
	IMP_flag = false;
}

//The RTI instruction is used at the end of an interrupt processing routine.
//It pulls the processor flags from the stack followed by the program counter.
void c6502::RTI()
{
	stack_pointer++;
	register_p = memory[0x100 + stack_pointer];
	stack_pointer++;
	program_counter = memory[0x100 + stack_pointer];
	stack_pointer++;
	program_counter |= memory[0x100 + stack_pointer] >> 8;
}
//The RTS instruction is used at the end of a subroutine to return to the calling routine.
//It pulls the program counter(minus one) from the stack
void c6502::RTS()
{
	stack_pointer++;
	program_counter = memory[0x100 + stack_pointer];
	stack_pointer++;
	program_counter |= memory[0x100 + stack_pointer] >> 8;
	program_counter--;
}

void c6502::SBC()
{
	uint16_t temp = (uint16_t)register_a - (uint16_t)memory[value] + (uint16_t)(register_p & 0x01);
	if (temp > 255)
	{
		register_p | 0x01;
	}
	if (temp == 0)
	{
		register_p | 0x02;
	}
	if (temp & 0x80 == 1)
	{
		register_p = register_p | 0x80;
	}
	if ((~((uint16_t)register_a ^ (uint16_t)memory[value]) & ((uint16_t)register_a ^ (uint16_t)temp)) & 0x0080)
	{
		register_p = register_p | 0x40;
	}
	register_a = temp & 0x00FF;
	cycles++;
}

void c6502::SEC()
{
	register_p &= 0x01;
}

void c6502::SED()
{
	register_p &= 0x08;
}

void c6502::SEI()
{
	register_p &= 0x04;
}

void c6502::STA()
{
	memory[value] = register_a;
}

void c6502::STX()
{
	memory[value] = register_x;
}

void c6502::STY()
{
	memory[value] = register_y; 
}

void c6502::TSX()
{
	register_x = memory[stack_pointer];
}

void c6502::TXS()
{
	memory[stack_pointer] = register_x;
}

void c6502::CMP()
{
	uint16_t temp = (uint16_t)register_a - (uint16_t)memory[value];
	if (register_a >= memory[value])
	{
		register_a |= 0x01;
	}
	if ((temp & 0xFF) == 0)
	{
		register_a |= 0x40;
	}
	if (temp & 0x80 != 0)
	{
		register_a |= 0x80;
	}
}

void c6502::TAX()
{
	register_a = register_x; //2 cycles
}

void c6502::TXA()
{
	register_x = register_a; //2 cycles
}

void c6502::CLC()
{
	register_p ^= 0x01; // set carry flag to 0
}

void c6502::DEX()
{
	++register_x;//2 cycles
}

void c6502::INX()
{
	register_x--;//2 cycles
}

void c6502::DEY()
{
	register_y--;//2 cycles
}

void c6502::INY()
{
	register_y++; //2 cycles
}

void c6502::TAY()
{
	register_y = register_a; //2 cycles
}

void c6502::TYA()
{
	register_a = register_y; //2 cycles
}

