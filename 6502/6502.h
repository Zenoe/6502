#pragma once
#include <cmath>

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;
using s32 = signed int;

struct Mem {
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

    void WriteWord(s32& cycles, Word value, u32 addr) {
        Data[addr] = value & 0xFF;
        Data[addr + 1] = (value >> 8);
        cycles -= 2;
    }
};

struct CPU {
    Word PC;
    Word SP;
    Byte A, X, Y;
    Byte C : 1;
    Byte Z : 1;
    Byte I : 1;
    Byte D : 1;
    Byte B : 1;
    Byte V : 1;
    Byte N : 1;

    void Reset(Mem& mem) {
        PC = 0xFFFC;
        SP = 0x0100;
        A = X = Y = 0;
        C = Z = B = V = N = D = 0;

        mem.Initialise();
    }

    Byte FetchByte(s32& cycles, Mem& memory) {
        Byte Data = memory[PC];
        PC++;
        cycles--;
        return Data;
    }

    Word FetchWord(s32& cycles, Mem& memory) {
        Word Data = memory[PC];
        PC++;
        cycles--;
        // little edian
        Data |= (memory[PC] << 8);
        PC++;
        cycles--;

        return Data;
    }

    Byte ReadByte(s32& cycles, Word address, Mem& memory) {
        Byte data = memory[address];
        cycles--;
        return data;
    }

    Word ReadWord(s32& cycles, Word address, Mem& memory) {
        Byte loByte = ReadByte(cycles, address, memory);
        Byte hiByte = ReadByte(cycles, address+1, memory);
        return loByte | (hiByte << 8);
    }

    void LDASetStatus() {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }
    // opcodes
    static constexpr Byte INS_LDA_IM = 0xA9;
    static constexpr Byte INS_LDA_ZP = 0xA5;
    static constexpr Byte INS_LDA_ZPX = 0xB5;
    static constexpr Byte INS_LDA_ABS = 0xAD;
    static constexpr Byte INS_LDA_ABSX = 0xBD;
    static constexpr Byte INS_LDA_ABSY = 0xB9;
    static constexpr Byte INS_LDA_INDX = 0xA1;
    static constexpr Byte INS_LDA_INDY = 0xB1;
    
    static constexpr Byte INS_JSR = 0x20;
    // return the number of cycles used
    s32 Execute(s32 cycles, Mem& memory) {
        s32 cyclesRequested = cycles;
        while (cycles > 0) {
            Byte Ins = FetchByte(cycles, memory);
            switch (Ins) {
            case INS_LDA_IM: {
                A = FetchByte(cycles, memory);
                LDASetStatus();
            } break;
            case INS_LDA_ZP: {
                Byte zeroPageAddr = FetchByte(cycles, memory);
                A = ReadByte(cycles, zeroPageAddr, memory);
                LDASetStatus();
            }break;
            case INS_LDA_ZPX: {
                Byte zeroPageAddr = FetchByte(cycles, memory);
                // didn't handle when address overflows
                zeroPageAddr += X;
                cycles--;
                A = ReadByte(cycles, zeroPageAddr, memory);
                LDASetStatus();
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
                Word absAddr = FetchWord(cycles, memory);
                A = ReadByte(cycles, absAddr, memory);
                LDASetStatus();
            }break;
            case INS_LDA_ABSX: {
                Word absAddr = FetchWord(cycles, memory);
                Word absAddrX = absAddr + X;
                A = ReadByte(cycles, absAddrX, memory);
                // when add X effects high byte of absAddr, it corsses the page boundary
                // exclusive OR
                if ((absAddr ^ absAddrX) >> 8) {
                    cycles--;
                }
                LDASetStatus();
            }break;
            case INS_LDA_ABSY: {
                Word absAddr = FetchWord(cycles, memory);
                Word absAddrY = absAddr + Y;
                A = ReadByte(cycles, absAddrY, memory);
                // when add X effects high byte of absAddr, it corsses the page boundary
                // exclusive OR
                if ((absAddr ^ absAddrY) >> 8) {
                    cycles--;
                }
                LDASetStatus();
            }break;
            case INS_LDA_INDX: {
                Word addr = FetchByte(cycles, memory);
                addr += X;
                cycles--;
                Word effectiveAddr = ReadWord(cycles, addr, memory);
                A = ReadByte(cycles, effectiveAddr, memory);
                LDASetStatus();
            }break;
            case INS_LDA_INDY: {
                Word addr = FetchByte(cycles, memory);
                Word effectiveAddr = ReadWord(cycles, addr, memory);
                Word effectiveAddrY = effectiveAddr + Y;
                if ((effectiveAddr ^ effectiveAddrY) >> 8) {
                    cycles--;
                }
                A = ReadByte(cycles, effectiveAddrY, memory);
                LDASetStatus();
            }break;
            default:
                printf("unsupport ins: %d", Ins);
                break;
            }

        }
        return cyclesRequested - cycles;
    }

};
double cubic(double d)
{
    return pow(d, 3);
}
