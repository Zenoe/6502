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
			//
			memory[i] = program[byteIdx++];
		}
	}
	return loadAddr;
}

s32 CPU::Execute(s32 cycles, Mem& memory) {
	s32 cyclesRequested = cycles;
	auto LoadReg = [&cycles, &memory, this](Word addr, Byte& regValue) {
		regValue = ReadByte(cycles, addr, memory);
		LDRegSetStatus(regValue);
	};
	auto LogicalOpA = [&cycles, &memory, this](Word addr, ELogicalOp logicalOp) {
		A = DoLogicalOp(A, ReadByte(cycles, addr, memory), logicalOp);
		LDRegSetStatus(A);
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
			cycles-=2;
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
			PushByte2Stack(cycles, memory, PS);
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

		}break;
		case INS_INY: {

		}break;
		case INS_DEY: {

		}break;
		case INS_DEX: {

		}break;
		case INS_DEC_ZP: {

		}break;
		case INS_DEC_ZPX: {

		}break;
		case INS_DEC_ABS: {

		}break;
		case INS_DEC_ABSX: {

		}break;
		case INS_INC_ZP: {

		}break;
		case INS_INC_ZPX: {

		}break;
		case INS_INC_ABS: {

		}break;
		case INS_INC_ABSX: {

		}break;
		case INS_BEQ: {

		}break;
		case INS_BNE: {

		}break;
		case INS_BCS: {

		}break;
		case INS_BCC: {

		}break;
		case INS_BMI: {

		}break;
		case INS_BPL: {

		}break;
		case INS_BVC: {

		}break;
		case INS_BVS: {

		}break;
		case INS_CLC: {

		}break;
		case INS_SEC: {

		}break;
		case INS_CLD: {

		}break;
		case INS_SED: {

		}break;
		case INS_CLI: {

		}break;
		case INS_SEI: {

		}break;
		case INS_CLV: {

		}break;
		case INS_ADC: {

		}break;
		case INS_ADC_ZP: {

		}break;
		case INS_ADC_ZPX: {

		}break;
		case INS_ADC_ABS: {

		}break;
		case INS_ADC_ABSX: {

		}break;
		case INS_ADC_ABSY: {

		}break;
		case INS_ADC_INDX: {

		}break;
		case INS_ADC_INDY: {

		}break;
		case INS_SBC: {

		}break;
		case INS_SBC_ABS: {

		}break;
		case INS_SBC_ZP: {

		}break;
		case INS_SBC_ZPX: {

		}break;
		case INS_SBC_ABSX: {

		}break;
		case INS_SBC_ABSY: {

		}break;
		case INS_SBC_INDX: {

		}break;
		case INS_SBC_INDY: {

		}break;
		case INS_CMP: {

		}break;
		case INS_CMP_ZP: {

		}break;
		case INS_CMP_ZPX: {

		}break;
		case INS_CMP_ABS: {

		}break;
		case INS_CMP_ABSX: {

		}break;
		case INS_CMP_ABSY: {

		}break;
		case INS_CMP_INDX: {

		}break;
		case INS_CMP_INDY: {

		}break;
		case INS_CPX: {

		}break;
		case INS_CPY: {

		}break;
		case INS_CPX_ZP: {

		}break;
		case INS_CPY_ZP: {

		}break;
		case INS_CPX_ABS: {

		}break;
		case INS_CPY_ABS: {

		}break;
		case INS_ASL: {

		}break;
		case INS_ASL_ZP: {

		}break;
		case INS_ASL_ZPX: {

		}break;
		case INS_ASL_ABS: {

		}break;
		case INS_ASL_ABSX: {

		}break;
		case INS_LSR: {

		}break;
		case INS_LSR_ZP: {

		}break;
		case INS_LSR_ZPX: {

		}break;
		case INS_LSR_ABS: {

		}break;
		case INS_LSR_ABSX: {

		}break;
		case INS_ROL: {

		}break;
		case INS_ROL_ZP: {

		}break;
		case INS_ROL_ZPX: {

		}break;
		case INS_ROL_ABS: {

		}break;
		case INS_ROL_ABSX: {

		}break;
		case INS_ROR: {

		}break;
		case INS_ROR_ZP: {

		}break;
		case INS_ROR_ZPX: {

		}break;
		case INS_ROR_ABS: {

		}break;
		case INS_ROR_ABSX: {

		}break;
		case INS_NOP: {

		}break;
		case INS_BRK: {

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
