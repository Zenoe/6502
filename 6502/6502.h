#pragma once
#include <cmath>
#include <stdio.h>
namespace m6502 {
	using Byte = unsigned char;
	using Word = unsigned short;
	using u32 = unsigned int;
	using s32 = signed int;
    struct Mem;
    struct CPU;
}
struct m6502::Mem {
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];
    void Initialise() {
        for (u32 i = 0; i < MAX_MEM; i++) {
            Data[i] = 0;
        }
    }
    Byte operator [](u32 address) const {
        // assert here address < MAX_MEM
        return Data[address];
    }

    Byte& operator [](u32 address) {
        // assert here address < MAX_MEM
        return Data[address];
    }
};

struct m6502::CPU {
    Word PC;
    Byte SP;
    Byte A, X, Y;
    Byte C : 1;
    Byte Z : 1;
    Byte I : 1;
    Byte D : 1;
    Byte B : 1;
    Byte V : 1;
    Byte N : 1;
    // opcodes
    static constexpr Byte INS_LDA_IM = 0xA9;
    static constexpr Byte INS_LDA_ZP = 0xA5;
    static constexpr Byte INS_LDA_ZPX = 0xB5;
    static constexpr Byte INS_LDA_ABS = 0xAD;
    static constexpr Byte INS_LDA_ABSX = 0xBD;
    static constexpr Byte INS_LDA_ABSY = 0xB9;
    static constexpr Byte INS_LDA_INDX = 0xA1;
    static constexpr Byte INS_LDA_INDY = 0xB1;
    
    // LDX
    static constexpr Byte INS_LDX_IM = 0xA2;
    static constexpr Byte INS_LDX_ZP = 0xA6;
    static constexpr Byte INS_LDX_ZPY = 0xB6; //
    static constexpr Byte INS_LDX_ABS = 0xAE;
    static constexpr Byte INS_LDX_ABSY = 0xBE;
    // LDY
    static constexpr Byte INS_LDY_IM = 0xA0;
    static constexpr Byte INS_LDY_ZP = 0xA4;
    static constexpr Byte INS_LDY_ZPX = 0xB4; //
    static constexpr Byte INS_LDY_ABS = 0xAC;
    static constexpr Byte INS_LDY_ABSX = 0xBC;

    // store
    static constexpr Byte INS_STA_ZP = 0x85;
    static constexpr Byte INS_STA_ZPX = 0x95;
    static constexpr Byte INS_STA_ABS = 0x8D;
    static constexpr Byte INS_STA_ABSX = 0x9D;
    static constexpr Byte INS_STA_ABSY = 0x99;
    static constexpr Byte INS_STA_INDX = 0x81;
    static constexpr Byte INS_STA_INDY = 0x91;

    static constexpr Byte INS_STX_ZP = 0x86;
    static constexpr Byte INS_STX_ZPY = 0x96;
    static constexpr Byte INS_STX_ABS = 0x8E;

    static constexpr Byte INS_STY_ZP = 0x84;
    static constexpr Byte INS_STY_ZPX = 0x94;
    static constexpr Byte INS_STY_ABS = 0x8C;

    static constexpr Byte INS_JSR = 0x20;
    static constexpr Byte INS_RTS = 0x60;

    void Reset(Mem& mem);
    void Reset(Word ResetVector, Mem& memory);

    Byte FetchByte(s32& cycles, const Mem& memory);
    Word FetchWord(s32& cycles, const Mem& memory);
    Byte ReadByte(s32& cycles, Word address,const Mem& memory);
    Word ReadWord(s32& cycles, Word address,const Mem& memory);

    void WriteByte(const Byte& regVal, const Word& addr, s32& cycles, Mem& memory);
    void WriteWord(Word value, s32& cycles, Word addr, Mem& memory);

	void LDRegSetStatus(const Byte & effectedRegVal);

    Word AddrZeroPage(s32& cycles, const Mem& memory);
    Word AddrZeroPageX(s32& cycles, const Mem& memory);
    Word AddrZeroPageY(s32& cycles, const Mem& memory);
    Word AddrAbs(s32& cycles, const Mem& memory);
    Word AddrAbsX(s32& cycles, const Mem& memory);
    Word AddrAbsY(s32& cycles, const Mem& memory);

	// take 5 cycles
    Word AddrAbsX_5(s32& cycles, const Mem& memory);
    Word AddrAbsY_5(s32& cycles, const Mem& memory);
    Word AddrIndirectX(s32& cycles, const Mem& memory);
    Word AddrIndirectY(s32& cycles, const Mem& memory);
    Word AddrIndirectY_6(s32& cycles, const Mem& memory);

    // 6502 supports a 256 byte stack fixed btw memory locations $0100 and $01FF.
    // a special reg SP is used to keep track of the next free byte of stack space.
    // pushing a value on to the stack causes the value to be stored at the current location
    // and then stack pointer is post decremented. Pull operations reverse this procedure
    Word SPToAddress() const;
    void PushWord2Stack(s32& cycles, Mem& memory, Word val);
    void PushByte2Stack(s32& cycles, Mem& memory, Byte val);
    Word PopWordFromStack(s32& cycles, Mem& memory);
    s32 Execute(s32 cycles, Mem& memory);

};
