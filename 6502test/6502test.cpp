// 6502test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "gtest/gtest.h"
#include "6502.h"

class M6502Test1 : public testing::Test {

public:
    Mem mem;
    CPU cpu;
    virtual void SetUp() {
        cpu.Reset(mem );
    }

    virtual void TearDown() {

    }
};

static void VerifyUnmodifiedFlagsFromLDA(const CPU& cpu, const CPU& cpuCopy) {
    EXPECT_EQ(cpu.C, cpuCopy.C);
    EXPECT_EQ(cpu.I, cpuCopy.I);
    EXPECT_EQ(cpu.D, cpuCopy.D);
    EXPECT_EQ(cpu.B, cpuCopy.B);
    EXPECT_EQ(cpu.V, cpuCopy.V);
}

TEST_F(M6502Test1, DoNothingWhenPassIn0Cycles) {
    constexpr s32 cycles = 0;
    s32 cyclesUsed = cpu.Execute(cycles, mem);
    EXPECT_EQ(cyclesUsed, 0);
}

TEST_F(M6502Test1, BadIns) {
}

TEST_F(M6502Test1, LDAImmediateAffect0Flag) {
    cpu.A = 0x44;
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x0;

    s32 cyclesUsed = cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x0); 
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
};

TEST_F(M6502Test1, CPUCanExecuteMoreCyclesThanRequestedByIns) {
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x84;
    s32 cyclesUsed = cpu.Execute(1, mem);

    // fixed me might be not correct
    EXPECT_EQ(cyclesUsed, 2);
}

TEST_F(M6502Test1, LDAImmediateLoadValIntoAReg) {
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x84;

    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x84); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);// 0x84 == 0b10000100
    EXPECT_EQ(cyclesUsed, 2);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDA0PageLoadValIntoAReg) {

    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x37;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(3, mem);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cyclesUsed, 3);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDA0PageXLoadValIntoAReg) {

    // given:
    cpu.X = 5;

    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0047] = 0x37;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(4, mem);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cyclesUsed, 4);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDA0PageXLoadValIntoARegWhenItWraps) {

    // given:
    cpu.X = 0xFF;

    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x80;
    mem[0x007F] = 0x37;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(4, mem);
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDAAbsLoadValIntoAReg) {
    mem[0xFFFC] = CPU::INS_LDA_ABS;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44;
    mem[0x4480] = 0x37;

    constexpr s32 expected_cycles = 4;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.A, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDAAbsXLoadValIntoAReg) {
    cpu.X = 1;
    mem[0xFFFC] = CPU::INS_LDA_ABSX;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44;
    mem[0x4481] = 0x37;

    constexpr s32 expected_cycles = 4;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.A, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDAAbsXLoadValIntoARegWhenItCrossesAPage) {
    cpu.X = 0x1;
    mem[0xFFFC] = CPU::INS_LDA_ABSX;
    mem[0xFFFD] = 0xFF;
    mem[0xFFFE] = 0x44; //0x44FF
    mem[0x4500] = 0x37; // 0x44FF+0x1 crosses page boundry

    constexpr s32 expected_cycles = 5;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.A, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDAAbsYLoadValIntoAReg) {
    cpu.Y = 1;
    mem[0xFFFC] = CPU::INS_LDA_ABSY;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44;
    mem[0x4481] = 0x37;

    constexpr s32 expected_cycles = 4;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.A, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDAIndirectXLoadValIntoAReg) {
    cpu.X = 0x04;
    mem[0xFFFC] = CPU::INS_LDA_INDX;
    mem[0xFFFD] = 0x02;
    mem[0x0006] = 0x00; // 0x04+0x02
    mem[0x0007] = 0x80;
    mem[0x8000] = 0x37;

    constexpr s32 expected_cycles = 6;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.A, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDAIndirectYLoadValIntoAReg) {
    cpu.Y = 0x04;
    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0xFFFD] = 0x02;
    mem[0x0002] = 0x00; 
    mem[0x0003] = 0x80;
    mem[0x8004] = 0x37;

    constexpr s32 expected_cycles = 5;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.A, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

TEST_F(M6502Test1, LDAIndirectYLoadValIntoARegWhenItCrossesAPage) {
    cpu.Y = 0xFF;
    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0xFFFD] = 0x02;
    mem[0x0002] = 0x02; 
    mem[0x0003] = 0x80;
    mem[0x8101] = 0x37;

    constexpr s32 expected_cycles = 6;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.A, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
};

