#include "6502.h"
using namespace m6502;

void CPU::Reset(Mem& mem) {
	Reset(0xFFFC, mem);
}
void CPU::Reset(Word ResetVector, Mem& memory)
{
	PC = ResetVector;
	SP = 0xFF;// pushing to stack leads to SP decrement
	C = I = Z = B = V = N = D = 0;
	//Flag.C = Flag.Z = Flag.I = Flag.D = Flag.B = Flag.V = Flag.N = 0;
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
	Z = (effectedRegVal == 0);
	N = (effectedRegVal & 0b10000000) > 0;
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

void CPU::PushByte2Stack(s32& cycles, Mem& memory, Byte val) {
	WriteByte(val, SPToAddress(), cycles, memory);
	SP--;
}

s32 CPU::Execute(s32 cycles, Mem& memory) {
	s32 cyclesRequested = cycles;
	auto LoadReg = [&cycles, &memory, this](Word addr, Byte& regValue) {
		regValue = ReadByte(cycles, addr, memory);
		LDRegSetStatus(regValue);
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
		case INS_JSR: {
			Word subRoutineAddr = FetchWord(cycles, memory);
			// pushes the address (minus one) of the return point on to the stack
			// and then sets the program counter to the target memory address.
			PushWord2Stack(cycles, memory, PC - 1);
			PC = subRoutineAddr;
			cycles--;
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
		default:
			printf("unsupport ins: %d", Ins);
			break;
		}
	}
	return cyclesRequested - cycles;
}
