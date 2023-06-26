#include "6502.h"
using namespace m6502;

void CPU::Reset(Mem& mem) {
	PC = 0xFFFC;
	SP = 0x0100;
	A = X = Y = 0;
	C = Z = B = V = N = D = 0;

	mem.Initialise();
}

Byte CPU::FetchByte(s32& cycles, const Mem& memory) {
	Byte Data = memory[PC];
	PC++;
	cycles--;
	return Data;
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
	//Z = (A == 0);
	//N = (A & 0b10000000) > 0;
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
			memory.WriteWord(cycles, PC - 1, SP);
			SP += 2; // add 2 not 1
			PC = subRoutineAddr;
			cycles--;
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
			Word addr = FetchByte(cycles, memory);
			addr += X;
			cycles--;
			Word effectiveAddr = ReadWord(cycles, addr, memory);
			LoadReg(effectiveAddr, A);
		}break;
		case INS_LDA_INDY: {
			Word addr = FetchByte(cycles, memory);
			Word effectiveAddr = ReadWord(cycles, addr, memory);
			Word effectiveAddrY = effectiveAddr + Y;
			if ((effectiveAddr ^ effectiveAddrY) >> 8) {
				cycles--;
			}
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

		}break;
		case INS_STA_ZPX: {

		}break;
		case INS_STA_ABS: {

		}break;
		case INS_STA_ABSX: {

		}break;
		case INS_STA_ABSY: {

		}break;
		case INS_STA_INDX: {

		}break;
		case INS_STA_INDY: {

		}break;
		case INS_STX_ZP: {

		}break;
		case INS_STX_ZPY: {

		}break;
		case INS_STX_ABS: {

		}break;
		case INS_STY_ZP: {

		}break;
		case INS_STY_ZPX: {

		}break;
		case INS_STY_ABS: {

		}break;
		default:
			printf("unsupport ins: %d", Ins);
			break;
		}
	}
	return cyclesRequested - cycles;
}
