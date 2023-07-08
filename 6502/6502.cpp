#include "6502.h"
using namespace m6502;

void CPU::Reset(Mem& mem) {
	Reset(0xFFFC, mem);
}
void CPU::Reset(Word ResetVector, Mem& memory)
{
	PC = ResetVector;
	SP = 0xFF;// pushing to stack leads to SP decrement
	Flag.C = Flag.Z = Flag.I = Flag.D = Flag.B = Flag.V = Flag.N = 0;
	A = X = Y = 0;
	memory.Initialise();
}

Byte CPU::FetchByte(s32& cycles, const Mem& memory) {
	Byte Data = memory[PC];
	PC++;
	cycles--;
	return Data;
}

void CPU::WriteByte(const Byte& regVal, const Word& addr, s32& cycles, Mem& memory) {
	memory[addr] = regVal;
	cycles--;
}
void CPU::WriteWord(Word value, s32& cycles, Word addr, Mem& memory) {
	memory[addr] = value & 0xFF;
	memory[addr + 1] = (value >> 8);
	cycles -= 2;
}

Word CPU::FetchWord(s32& cycles, const Mem& memory) {
	Word Data = memory[PC];
	PC++;
	cycles--;
	// little edian
	Data |= (memory[PC] << 8);
	PC++;
	cycles--;

	return Data;
}

Byte CPU::ReadByte(s32& cycles, Word address, const Mem& memory) {
	Byte data = memory[address];
	cycles--;
	return data;
}

Word CPU::ReadWord(s32& cycles, Word address, const Mem& memory) {
	Byte loByte = ReadByte(cycles, address, memory);
	Byte hiByte = ReadByte(cycles, address + 1, memory);
	return loByte | (hiByte << 8);
}

void CPU::LDRegSetStatus(const Byte& effectedRegVal) {
	Flag.Z = (effectedRegVal == 0);
	Flag.N = (effectedRegVal & 0b10000000) > 0;
}

Word CPU::AddrZeroPage(s32& cycles, const Mem& memory) {
	return FetchByte(cycles, memory);
}
Word CPU::AddrZeroPageX(s32& cycles, const Mem& memory) {
	Byte zeroPageAddr =  FetchByte(cycles, memory);
	// didn't handle when address overflows
	zeroPageAddr += X;
	cycles--;
	return zeroPageAddr;
}
Word CPU::AddrZeroPageY(s32& cycles, const Mem& memory) {
	Byte zeroPageAddr =  FetchByte(cycles, memory);
	zeroPageAddr += Y;
	cycles--;
	return zeroPageAddr;
}

Word CPU::AddrAbs(s32& cycles, const Mem& memory) {
	Word absAddr = FetchWord(cycles, memory);
	return absAddr;
}
Word CPU::AddrAbsX(s32& cycles, const Mem& memory) {
	Word absAddr = FetchWord(cycles, memory);
	Word absAddrX = absAddr + X;
	// when add X effects high byte of absAddr, it corsses the page boundary
	// exclusive OR
	if ((absAddr ^ absAddrX) >> 8) {
		cycles--;
	}
	return absAddrX;
}
Word CPU::AddrAbsY(s32& cycles, const Mem& memory) {
	Word absAddr = FetchWord(cycles, memory);
	Word absAddrY = absAddr + Y;
	// when add X effects high byte of absAddr, it corsses the page boundary
	// exclusive OR
	if ((absAddr ^ absAddrY) >> 8) {
		cycles--;
	}
	return absAddrY;
}
Word CPU::AddrAbsX_5(s32& cycles, const Mem& memory) {
	Word absAddr = FetchWord(cycles, memory);
	Word absAddrX = absAddr + X;
	cycles--;
	return absAddrX;
}
Word CPU::AddrAbsY_5(s32& cycles, const Mem& memory) {
	Word absAddr = FetchWord(cycles, memory);
	Word absAddrY = absAddr + Y;
	cycles--;
	return absAddrY;
}
Word CPU::AddrIndirectX(s32& cycles, const Mem& memory) {
	Word addr = FetchByte(cycles, memory);
	addr += X;
	cycles--;
	Word effectiveAddr = ReadWord(cycles, addr, memory);
	return effectiveAddr;
}

Word CPU::AddrIndirectY(s32& cycles, const Mem& memory) {
	Word addr = FetchByte(cycles, memory);
	Word effectiveAddr = ReadWord(cycles, addr, memory);
	Word effectiveAddrY = effectiveAddr + Y;
	if ((effectiveAddr ^ effectiveAddrY) >> 8) {
		cycles--;
	}
	return effectiveAddrY;
}
Word CPU::AddrIndirectY_6(s32& cycles, const Mem& memory) {
	Word addr = FetchByte(cycles, memory);
	Word effectiveAddr = ReadWord(cycles, addr, memory);
	Word effectiveAddrY = effectiveAddr + Y;
	cycles--;
	return effectiveAddrY;
}

Word CPU::SPToAddress() const {
	return 0x100 | SP;
}

void CPU::PushWord2Stack(s32& cycles, Mem& memory, Word val) {
	WriteByte(val >> 8, SPToAddress(), cycles, memory);
	SP--;
	WriteByte(val & 0xFF, SPToAddress(), cycles, memory);
	SP--;
}

Word CPU::PopWordFromStack(s32& cycles, Mem& memory) {
	Word retVal = ReadWord(cycles, SPToAddress()+1, memory);
	SP+=2;
	cycles--;
	return retVal;
}

Byte CPU::PopByteFromStack(s32& cycles, Mem& memory) {
	Byte retVal = ReadByte(cycles, SPToAddress() + 1, memory);
	SP += 1;
	cycles--;
	return retVal;
}

void CPU::PushByte2Stack(s32& cycles, Mem& memory, Byte val) {
	WriteByte(val, SPToAddress(), cycles, memory);
	SP--;
}

Word CPU::LoadProg(const Byte* program, u32 byteCount, Mem& memory) {
	Word loadAddr = 0;
	if (program) {
		u32 byteIdx = 0;
		Byte lo = program[byteIdx++];
		Byte hi = program[byteIdx++];
		loadAddr = lo | (hi << 8);
		for (Word i = loadAddr; i < loadAddr+byteCount-2; i++)
		{
			// memcpy?
			memory[i] = program[byteIdx++];
		}
	}
	return loadAddr;
}

s32 CPU::Execute(s32 cycles, Mem& memory) {
	s32 cyclesRequested = cycles;
	auto PushPSToStack = [&cycles, &memory, this]()
	{
		Byte PSStack = PS | BreakFlagBit | UnusedFlagBit;
		PushByte2Stack(cycles, memory, PSStack);
	};

	auto LoadReg = [&cycles, &memory, this](Word addr, Byte& regValue) {
		regValue = ReadByte(cycles, addr, memory);
		LDRegSetStatus(regValue);
	};
	auto LogicalOpA = [&cycles, &memory, this](Word addr, ELogicalOp logicalOp) {
		A = DoLogicalOp(A, ReadByte(cycles, addr, memory), logicalOp);
		LDRegSetStatus(A);
	};
	auto ADC = [&cycles, &memory, this](Byte operand) {
		const bool sameSigned = !((A ^ operand) & NegativeFlagBit);
		Byte oldA = A;
		Word sum = A + operand + Flag.C;
		A = sum & 0xFF;
		LDRegSetStatus(A);
		Flag.C = sum > 0xFF;
		if (sameSigned) {
			Byte sumResult = A;
			// not same signed btw sumResult and operand
			Flag.V = ((sumResult ^ operand) & NegativeFlagBit);
		}
		else {
			Flag.V = false;
		}
	};
	auto SBC = [&ADC](Byte operand) {
		// set carry to be true
		// A-B ==> A + (-B)
		// -B = ~B + 1   (1 is added by setting carry bit true)
		ADC(~operand);
	};
	auto BranchIf = [&cycles, &memory, this](bool expectValue, bool target) {
		if (target == expectValue) {
			s8 offset = static_cast<s8> (FetchByte(cycles, memory));
			const Word oldPC = PC;
			PC += offset;
			cycles--;
			if ((oldPC >> 8) != (PC >> 8)) {
				cycles--;
			}
		}
	};
	auto RegCmp = [&cycles, &memory, this](Byte operand, Byte regVal) {
		Byte res = regVal - operand;
		//Flag.C = (res >= 0); // wrong! when the result of subtraction is negative, res is converted to unsigned int, that is > 0
		Flag.C = (regVal >= operand);
		Flag.Z = (res == 0);
		Flag.N = ((res & NegativeFlagBit) > 0);
	};
	auto ASL = [&cycles, this](Byte Operand) -> Byte
	{
		Flag.C = (Operand & NegativeFlagBit) > 0;
		Byte Result = Operand << 1;
		LDRegSetStatus(Result);
		cycles--;
		return Result;
	};
	auto LSR = [&cycles, this](Byte Operand) -> Byte
	{
		Flag.C = (Operand & ZeroBit) > 0;
		Byte Result = Operand >> 1;
		LDRegSetStatus(Result);
		cycles--;
		return Result;
	};
	auto ROL = [&cycles, this](Byte operand)->Byte
	{
		Byte newBit0 = Flag.C ? ZeroBit : 0;
		Flag.C = (operand & NegativeFlagBit) > 0;
		Byte result = operand << 1;
		result = result | newBit0;
		cycles--;
		LDRegSetStatus(result);
		return result;
	};
	auto ROR = [&cycles, this](Byte operand)->Byte
	{
		Byte carry = Flag.C;
		Flag.C = (operand & ZeroBit) > 0;
		Byte result = operand >> 1;
		result = result | (carry << 7);
		LDRegSetStatus(result);
		cycles--;
		return result;
	};

	while (cycles > 0) {
		Byte Ins = FetchByte(cycles, memory);
		switch (Ins) {
		case INS_LDA_IM: {
			A = FetchByte(cycles, memory);
			LDRegSetStatus(A);
		} break;
		case INS_LDA_ZP: {
			Byte zeroPageAddr = FetchByte(cycles, memory);
			LoadReg(zeroPageAddr, A);
		}break;
		case INS_LDA_ZPX: {
			Byte zeroPageAddr = AddrZeroPageX(cycles, memory);
			LoadReg(zeroPageAddr, A);
		}break;
		case INS_RTS: {
			// return to the calling routine. it pulls the program counter(minus one) from the stack
			Word popAddr = PopWordFromStack(cycles, memory);
			PC = popAddr + 1;
			cycles -= 2;
		}break;
		case INS_LDA_ABS: {
			Word absAddr = AddrAbs(cycles, memory);
			LoadReg(absAddr, A);
		}break;
		case INS_LDA_ABSX: {
			Word absAddrX = AddrAbsX(cycles, memory);
			LoadReg(absAddrX, A);
		}break;
		case INS_LDA_ABSY: {
			Word absAddrY = AddrAbsY(cycles, memory);
			LoadReg(absAddrY, A);
		}break;
		case INS_LDA_INDX: {
			Word effectiveAddr = AddrIndirectX(cycles, memory);
			LoadReg(effectiveAddr, A);
		}break;
		case INS_LDA_INDY: {
			Word effectiveAddrY = AddrIndirectY(cycles, memory);
			LoadReg(effectiveAddrY, A);
		}break;
			// ldx
		case INS_LDX_IM: {
			X = FetchByte(cycles, memory);
			LDRegSetStatus(X);
		}break;
		case INS_LDX_ZP: {
			Byte zeroPageAddr = FetchByte(cycles, memory);
			LoadReg(zeroPageAddr, X);
		}break;
		case INS_LDX_ZPY: {
			Byte zeroPageAddr = AddrZeroPageY(cycles, memory);
			LoadReg(zeroPageAddr, X);
		}break;
		case INS_LDX_ABS: {
			Word absAddr = AddrAbs(cycles, memory);
			LoadReg(absAddr, X);
		}break;
		case INS_LDX_ABSY: {
			Word absAddrY = AddrAbsY(cycles, memory);
			LoadReg(absAddrY, X);
		}break;
			// ldy
		case INS_LDY_IM: {
			Y = FetchByte(cycles, memory);
			LDRegSetStatus(Y);
		}break;
		case INS_LDY_ZP: {
			Byte zeroPageAddr = FetchByte(cycles, memory);
			LoadReg(zeroPageAddr, Y);
		}break;
		case INS_LDY_ZPX: {
			Byte zeroPageAddr = AddrZeroPageX(cycles, memory);
			LoadReg(zeroPageAddr, Y);
		}break;
		case INS_LDY_ABS: {
			Word absAddr = AddrAbs(cycles, memory);
			LoadReg(absAddr, Y);
		}break;
		case INS_LDY_ABSX: {
			Word absAddrX = AddrAbsX(cycles, memory);
			LoadReg(absAddrX, Y);
		}break;
		case INS_STA_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			WriteByte(A, addr, cycles, memory);
		}break;
		case INS_STA_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			WriteByte(A, addr, cycles, memory);
		}break;
		case INS_STA_ABS: {
			Word addr = AddrAbs(cycles, memory);
			WriteByte(A, addr, cycles, memory);
		}break;
		case INS_STA_ABSX: {
			Word addr = AddrAbsX_5(cycles, memory);
			WriteByte(A, addr, cycles, memory);
		}break;
		case INS_STA_ABSY: {
			Word addr = AddrAbsY_5(cycles, memory);
			WriteByte(A, addr, cycles, memory);
		}break;
		case INS_STA_INDX: {
			Word addr = AddrIndirectX(cycles, memory);
			WriteByte(A, addr, cycles, memory);
		}break;
		case INS_STA_INDY: {
			Word addr = AddrIndirectY_6(cycles, memory);
			WriteByte(A, addr, cycles, memory);
		}break;
		case INS_STX_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			WriteByte(X, addr, cycles, memory);
		}break;
		case INS_STX_ZPY: {
			Word addr = AddrZeroPageY(cycles, memory);
			WriteByte(X, addr, cycles, memory);
		}break;
		case INS_STX_ABS: {
			Word addr = AddrAbs(cycles, memory);
			WriteByte(X, addr, cycles, memory);
		}break;
		case INS_STY_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			WriteByte(Y, addr, cycles, memory);
		}break;
		case INS_STY_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			WriteByte(Y, addr, cycles, memory);
		}break;
		case INS_STY_ABS: {
			Word addr = AddrAbs(cycles, memory);
			WriteByte(Y, addr, cycles, memory);
		}break;
		case INS_JSR: {
			Word subRoutineAddr = FetchWord(cycles, memory);
			// pushes the address (minus one) of the return point on to the stack
			// and then sets the program counter to the target memory address.
			PushWord2Stack(cycles, memory, PC - 1);
			PC = subRoutineAddr;
			cycles--;
		}break;
		case INS_JMP_ABS: {
			Word subRoutineAddr = FetchWord(cycles, memory);
			PC = subRoutineAddr;
		}break;
		case INS_JMP_IND: {
			Word addr = FetchWord(cycles, memory);
			PC = ReadWord(cycles, addr, memory);
		}break;
		case INS_TSX: {
			X = SP;
			cycles--;
			LDRegSetStatus(X);
		}break;
		case INS_TXS: {
			SP = X;
			cycles--;
		}break;
		case INS_PHA: {
			PushByte2Stack(cycles, memory, A);
		}break;
		case INS_PLA: {
			A = PopByteFromStack(cycles, memory);
			LDRegSetStatus(A);
			cycles--;
		}break;
		case INS_PHP: {
			// needs to set 4th,5th bit of status flag, which were documented in nes.dev
			PushPSToStack();
		}break;
		case INS_PLP: {
			PS = PopByteFromStack(cycles, memory);
			cycles--;
		}break;
		case INS_AND_IM: {
			A = A & FetchByte(cycles, memory);
			LDRegSetStatus(A);
		}break;
		case INS_AND_ZP: {
			Byte zeroPageAddr = FetchByte(cycles, memory);
			LogicalOpA(zeroPageAddr, ELogicalOp::And);
		}break;
		case INS_AND_ZPX: {
			Byte zeroPageAddrX = AddrZeroPageX(cycles, memory);
			LogicalOpA(zeroPageAddrX, ELogicalOp::And);
		}break;
		case INS_AND_ABS: {
			Word addr = AddrAbs(cycles, memory);
			LogicalOpA(addr, ELogicalOp::And);
		}break;
		case INS_AND_ABSX: {
			Word addr = AddrAbsX(cycles, memory);
			LogicalOpA(addr, ELogicalOp::And);
		}break;
		case INS_AND_ABSY: {
			Word addr = AddrAbsY(cycles, memory);
			LogicalOpA(addr, ELogicalOp::And);
		}break;
		case INS_AND_INDX: {
			Word addr = AddrIndirectX(cycles, memory);
			LogicalOpA(addr, ELogicalOp::And);
		}break;
		case INS_AND_INDY: {
			Word addr = AddrIndirectY(cycles, memory);
			LogicalOpA(addr, ELogicalOp::And);
		}break;

		case INS_ORA_IM: {
			A = DoLogicalOp( A , FetchByte(cycles, memory), ELogicalOp::Or);
			LDRegSetStatus(A);
		}break;
		case INS_ORA_ZP: {
			Byte zeroPageAddr = FetchByte(cycles, memory);
			LogicalOpA(zeroPageAddr, ELogicalOp::Or);
		}break;
		case INS_ORA_ZPX: {
			Byte zeroPageAddrX = AddrZeroPageX(cycles, memory);
			LogicalOpA(zeroPageAddrX, ELogicalOp::Or);
		}break;
		case INS_ORA_ABS: {
			Word addr = AddrAbs(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Or);
		}break;
		case INS_ORA_ABSX: {
			Word addr = AddrAbsX(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Or);
		}break;
		case INS_ORA_ABSY: {
			Word addr = AddrAbsY(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Or);
		}break;
		case INS_ORA_INDX: {
			Word addr = AddrIndirectX(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Or);
		}break;
		case INS_ORA_INDY: {
			Word addr = AddrIndirectY(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Or);
		}break;

		case INS_EOR_IM: {
			A = DoLogicalOp( A , FetchByte(cycles, memory), ELogicalOp::Eor);
			LDRegSetStatus(A);
		}break;
		case INS_EOR_ZP: {
			Byte zeroPageAddr = FetchByte(cycles, memory);
			LogicalOpA(zeroPageAddr, ELogicalOp::Eor);
		}break;
		case INS_EOR_ZPX: {
			Byte zeroPageAddrX = AddrZeroPageX(cycles, memory);
			LogicalOpA(zeroPageAddrX, ELogicalOp::Eor);
		}break;
		case INS_EOR_ABS: {
			Word addr = AddrAbs(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Eor);
		}break;
		case INS_EOR_ABSX: {
			Word addr = AddrAbsX(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Eor);
		}break;
		case INS_EOR_ABSY: {
			Word addr = AddrAbsY(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Eor);
		}break;
		case INS_EOR_INDX: {
			Word addr = AddrIndirectX(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Eor);
		}break;
		case INS_EOR_INDY: {
			Word addr = AddrIndirectY(cycles, memory);
			LogicalOpA(addr, ELogicalOp::Eor);
		}break;

		case INS_BIT_ZP: {
			Byte addr = FetchByte(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			//bool bitTestResult = A & val;
			Flag.Z = !(A & val);
			Flag.N = (val & NegativeFlagBit) != 0;
			Flag.V = (val & OverflowFlagBit) != 0;
		}break;
		case INS_BIT_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			Flag.Z = !(A & val);
			Flag.N = (val & NegativeFlagBit) != 0;
			Flag.V = (val & OverflowFlagBit) != 0;
		}break;
		case INS_TAX: {
			X = A;
			cycles--;
			LDRegSetStatus(X);
		}break;
		case INS_TAY: {
			Y = A;
			cycles--;
			LDRegSetStatus(Y);
		}break;
		case INS_TXA: {
			A = X;
			cycles--;
			LDRegSetStatus(A);
		}break;
		case INS_TYA: {
			A = Y;
			cycles--;
			LDRegSetStatus(A);
		}break;

		case INS_INX: {
			X += 1;
			cycles--;
			LDRegSetStatus(X);
		}break;
		case INS_INY: {
			Y += 1;
			cycles--;
			LDRegSetStatus(Y);
		}break;
		case INS_DEY: {
			Y -= 1;
			cycles--;
			LDRegSetStatus(Y);
		}break;
		case INS_DEX: {
			X -= 1;
			cycles--;
			LDRegSetStatus(X);
		}break;
		case INS_DEC_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val--;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;
		case INS_DEC_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val--;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;
		case INS_DEC_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val--;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;
		case INS_DEC_ABSX: {
			Word addr = AddrAbsX(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val--;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;
		case INS_INC_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val++;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;
		case INS_INC_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val++;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;
		case INS_INC_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val++;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;
		case INS_INC_ABSX: {
			Word addr = AddrAbsX(cycles, memory);
			Byte val = ReadByte(cycles, addr, memory);
			val++;
			cycles--;
			WriteByte(val, addr, cycles, memory);
			LDRegSetStatus(val);
		}break;

		case INS_BEQ: {
			BranchIf(true, Flag.Z);
		}break;
		case INS_BNE: {
			BranchIf(false, Flag.Z);
		}break;
		case INS_BCS: {
			BranchIf(true, Flag.C);
		}break;
		case INS_BCC: {
			BranchIf(false, Flag.C);
		}break;
		case INS_BMI: {
			BranchIf(true, Flag.N);
		}break;
		case INS_BPL: {
			BranchIf(false, Flag.N);
		}break;
		case INS_BVC: {
			BranchIf(false, Flag.V);
		}break;
		case INS_BVS: {
			BranchIf(true, Flag.V);
		}break;
		case INS_CLC:
		{
			Flag.C = false;
			cycles--;
		} break;
		case INS_SEC:
		{
			Flag.C = true;
			cycles--;
		} break;
		case INS_CLD:
		{
			Flag.D = false;
			cycles--;
		} break;
		case INS_SED:
		{
			Flag.D = true;
			cycles--;
		} break;
		case INS_CLI:
		{
			Flag.I = false;
			cycles--;
		} break;
		case INS_SEI:
		{
			Flag.I = true;
			cycles--;
		} break;
		case INS_CLV:
		{
			Flag.V = false;
			cycles--;
		} break;
		case INS_ADC: {
			Byte operand = FetchByte(cycles, memory);
			ADC(operand);
		}break;
		case INS_ADC_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			ADC(operand);
		}break;
		case INS_ADC_ABS:
		{
			Word Address = AddrAbs(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			ADC(Operand);
		} break;
		case INS_ADC_ABSX:
		{
			Word Address = AddrAbsX(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			ADC(Operand);
		} break;
		case INS_ADC_ABSY:
		{
			Word Address = AddrAbsY(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			ADC(Operand);
		} break;
		case INS_ADC_ZPX:
		{
			Word Address = AddrZeroPageX(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			ADC(Operand);
		} break;
		case INS_ADC_INDX:
		{
			Word Address = AddrIndirectX(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			ADC(Operand);
		} break;
		case INS_ADC_INDY:
		{
			Word Address = AddrIndirectY(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			ADC(Operand);
		} break;
		case INS_SBC:
		{
			Byte Operand = FetchByte(cycles, memory);
			SBC(Operand);
		} break;
		case INS_SBC_ABS:
		{
			Word Address = AddrAbs(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			SBC(Operand);
		} break;
		case INS_SBC_ZP:
		{
			Word Address = AddrZeroPage(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			SBC(Operand);
		} break;
		case INS_SBC_ZPX:
		{
			Word Address = AddrZeroPageX(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			SBC(Operand);
		} break;
		case INS_SBC_ABSX:
		{
			Word Address = AddrAbsX(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			SBC(Operand);
		} break;
		case INS_SBC_ABSY:
		{
			Word Address = AddrAbsY(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			SBC(Operand);
		} break;
		case INS_SBC_INDX:
		{
			Word Address = AddrIndirectX(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			SBC(Operand);
		} break;
		case INS_SBC_INDY:
		{
			Word Address = AddrIndirectY(cycles, memory);
			Byte Operand = ReadByte(cycles, Address, memory);
			SBC(Operand);
		} break;
		case INS_CMP: {
			Byte operand = FetchByte(cycles, memory);
			RegCmp(operand, A);
		}break;
		case INS_CMP_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, A);
		}break;
		case INS_CMP_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, A);
		}break;
		case INS_CMP_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, A);
		}break;
		case INS_CMP_ABSX: {
			Word addr = AddrAbsX(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, A);
		}break;
		case INS_CMP_ABSY: {
			Word addr = AddrAbsY(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, A);
		}break;
		case INS_CMP_INDX: {
			Word addr = AddrIndirectX(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, A);
		}break;
		case INS_CMP_INDY: {
			Word addr = AddrIndirectY(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, A);
		}break;
		case INS_CPX: {
			Byte operand = FetchByte(cycles, memory);
			RegCmp(operand, X);
		}break;
		case INS_CPY: {
			Byte operand = FetchByte(cycles, memory);
			RegCmp(operand, Y);
		}break;
		case INS_CPX_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, X);
		}break;
		case INS_CPY_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, Y);
		}break;
		case INS_CPX_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, X);
		}break;
		case INS_CPY_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			RegCmp(operand, Y);
		}break;
		case INS_ASL: {
			A = ASL(A);
		}break;
		case INS_ASL_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ASL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ASL_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ASL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ASL_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ASL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ASL_ABSX: {
			Word addr = AddrAbsX_5(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ASL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_LSR: {
			A = LSR(A);
		}break;
		case INS_LSR_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = LSR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_LSR_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = LSR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_LSR_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = LSR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_LSR_ABSX: {
			Word addr = AddrAbsX_5(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = LSR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROL: {
			A = ROL(A);
		}break;
		case INS_ROL_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROL_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROL_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROL_ABSX: {
			Word addr = AddrAbsX_5(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROL(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROR: {
			A = ROR(A);
		}break;
		case INS_ROR_ZP: {
			Word addr = AddrZeroPage(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROR_ZPX: {
			Word addr = AddrZeroPageX(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROR_ABS: {
			Word addr = AddrAbs(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_ROR_ABSX: {
			Word addr = AddrAbsX_5(cycles, memory);
			Byte operand = ReadByte(cycles, addr, memory);
			Byte result = ROR(operand);
			WriteByte(result, addr, cycles, memory);
		}break;
		case INS_NOP: {
			cycles--;
		}break;
		case INS_BRK: {
			//PushWord2Stack(cycles, memory, PC - 1);
		}break;
		case INS_RTI: {

		}break;

		default:
			printf("unsupport ins: %d", Ins);
			break;
		}
	}
	return cyclesRequested - cycles;
}
