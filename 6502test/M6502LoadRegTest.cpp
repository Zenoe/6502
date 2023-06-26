// 6502test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "M6502Test.h"

static void VerifyUnmodifiedFlagsFromLDA(const CPU& cpu, const CPU& cpuCopy) {
    EXPECT_EQ(cpu.C, cpuCopy.C);
    EXPECT_EQ(cpu.I, cpuCopy.I);
    EXPECT_EQ(cpu.D, cpuCopy.D);
    EXPECT_EQ(cpu.B, cpuCopy.B);
    EXPECT_EQ(cpu.V, cpuCopy.V);
}

// *Reg is a pointer to member variable
void M6502LoadRegTest::TestLoadRegImmediate(Byte opCodeToTest, Byte CPU::*RegToTest ) {
    mem[0xFFFC] = opCodeToTest;
    mem[0xFFFD] = 0x84;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(2, mem);

    EXPECT_EQ(cpu.*RegToTest, 0x84); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N);// 0x84 == 0b10000100
    EXPECT_EQ(cyclesUsed, 2);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

void M6502LoadRegTest::TestLoadReg0Page(Byte opCodeToTest, Byte CPU::*RegToTest ) {
    mem[0xFFFC] = opCodeToTest;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x37;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(3, mem);
    EXPECT_EQ(cpu.*RegToTest, 0x37);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cyclesUsed, 3);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

void M6502LoadRegTest::TestLoadReg0PageX(Byte OpcodeToTest, Byte m6502::CPU::* RegisterToTest) {
	cpu.X = 5;
	mem[0xFFFC] = OpcodeToTest;
	mem[0xFFFD] = 0x42;
	mem[0x0047] = 0x37;
	CPU CPUCopy = cpu;

	//when:
	s32 CyclesUsed = cpu.Execute(4, mem);

	//then:
	EXPECT_EQ(cpu.*RegisterToTest, 0x37);
	EXPECT_EQ(CyclesUsed, 4);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}
void M6502LoadRegTest::TestLoadReg0PageY(Byte OpcodeToTest, Byte m6502::CPU::* RegisterToTest) {
    // todo
	cpu.Y = 5;
	mem[0xFFFC] = OpcodeToTest;
	mem[0xFFFD] = 0x42;
	mem[0x0047] = 0x37;
	CPU CPUCopy = cpu;
	s32 CyclesUsed = cpu.Execute(4, mem);

	EXPECT_EQ(cpu.*RegisterToTest, 0x37);
	EXPECT_EQ(CyclesUsed, 4);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, CPUCopy);
}

void M6502LoadRegTest::TestAbsLoadReg(Byte opcodeToTest, Byte m6502::CPU::* regToTest) {
    mem[0xFFFC] = opcodeToTest;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44;
    mem[0x4480] = 0x37;

    cpu.Z = cpu.N = true;
    constexpr s32 expected_cycles = 4;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.*regToTest, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
void M6502LoadRegTest::TestAbsXLoadReg(Byte opcodeToTest, Byte m6502::CPU::* regToTest) {
	cpu.X = 1;

    cpu.Z = cpu.N = true;
	mem[0xFFFC] = opcodeToTest;
	mem[0xFFFD] = 0x80;
	mem[0xFFFE] = 0x44;
	mem[0x4481] = 0x37;

	constexpr s32 expected_cycles = 4;
	CPU cpuCopy = cpu;
	s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

	EXPECT_EQ(cpu.*regToTest, 0x37);
	EXPECT_EQ(cyclesUsed, expected_cycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

void M6502LoadRegTest::TestAbsYLoadReg(Byte opcodeToTest, Byte m6502::CPU::* regToTest) {
	cpu.Y = 1;
    cpu.Z = cpu.N = true;
	mem[0xFFFC] = opcodeToTest;
	mem[0xFFFD] = 0x80;
	mem[0xFFFE] = 0x44;
	mem[0x4481] = 0x37;

	constexpr s32 expected_cycles = 4;
	CPU cpuCopy = cpu;
	s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

	EXPECT_EQ(cpu.*regToTest, 0x37);
	EXPECT_EQ(cyclesUsed, expected_cycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}


void M6502LoadRegTest::TestAbsXLoadRegWhenItCrossesPage(Byte opcodeToTest, Byte m6502::CPU::* regToTest) {
    cpu.X = 0x1;
    mem[0xFFFC] = opcodeToTest;
    mem[0xFFFD] = 0xFF;
    mem[0xFFFE] = 0x44; //0x44FF
    mem[0x4500] = 0x37; // 0x44FF+0x1 crosses page boundry

    constexpr s32 expected_cycles = 5;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.*regToTest, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

void M6502LoadRegTest::TestAbsYLoadRegWhenItCrossesPage(Byte opcodeToTest, Byte m6502::CPU::* regToTest) {
    cpu.Y = 0x1;
    mem[0xFFFC] = opcodeToTest;
    mem[0xFFFD] = 0xFF;
    mem[0xFFFE] = 0x44; //0x44FF
    mem[0x4500] = 0x37; // 0x44FF+0x1 crosses page boundry

    constexpr s32 expected_cycles = 5;
    CPU cpuCopy = cpu;
    s32 cyclesUsed = cpu.Execute(expected_cycles, mem);

    EXPECT_EQ(cpu.*regToTest, 0x37); 
    EXPECT_EQ(cyclesUsed, expected_cycles); 
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502LoadRegTest, LDAImmediateLoadValIntoAReg) {
    TestLoadRegImmediate(CPU::INS_LDA_IM, &CPU::A);
};

TEST_F(M6502LoadRegTest, LDXImmediateLoadValIntoXReg) {
    TestLoadRegImmediate(CPU::INS_LDX_IM, &CPU::X);
};
TEST_F(M6502LoadRegTest, LDYImmediateLoadValIntoYReg) {
    TestLoadRegImmediate(CPU::INS_LDY_IM, &CPU::Y);
};
TEST_F(M6502LoadRegTest, DoNothingWhenPassIn0Cycles) {
    constexpr s32 cycles = 0;
    s32 cyclesUsed = cpu.Execute(cycles, mem);
    EXPECT_EQ(cyclesUsed, 0);
}

TEST_F(M6502LoadRegTest, LDAImmediateAffect0Flag) {
    cpu.A = 0x44;
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x0;

    s32 cyclesUsed = cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x0); 
    EXPECT_TRUE(cpu.Z);
    EXPECT_FALSE(cpu.N);
};

TEST_F(M6502LoadRegTest, CPUCanExecuteMoreCyclesThanRequestedByIns) {
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x84;
    s32 cyclesUsed = cpu.Execute(1, mem);

    // fixed me might be not correct
    EXPECT_EQ(cyclesUsed, 2);
}

TEST_F(M6502LoadRegTest, LDA0PageLoadValIntoAReg) {
    TestLoadReg0Page(CPU::INS_LDA_ZP, &CPU::A);
};

TEST_F(M6502LoadRegTest, LDX0PageLoadValIntoXReg) {
    TestLoadReg0Page(CPU::INS_LDX_ZP, &CPU::X);
};
TEST_F(M6502LoadRegTest, LDY0PageLoadValIntoYReg) {
    TestLoadReg0Page(CPU::INS_LDY_ZP, &CPU::Y);
};
TEST_F(M6502LoadRegTest, LDA0PageXLoadValIntoAReg) {
    TestLoadReg0PageX(CPU::INS_LDA_ZPX, &CPU::A);
};
TEST_F(M6502LoadRegTest, LDY0PageXLoadValIntoYReg) {
    TestLoadReg0PageX(CPU::INS_LDY_ZPX, &CPU::Y);
};
TEST_F(M6502LoadRegTest, LDX0PageYLoadValIntoYReg) {
    TestLoadReg0PageY(CPU::INS_LDX_ZPY, &CPU::X);
};

TEST_F(M6502LoadRegTest, LDA0PageXLoadValIntoARegWhenItWraps) {
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

TEST_F(M6502LoadRegTest, LDAAbsLoadValIntoAReg) {
    TestAbsLoadReg(CPU::INS_LDA_ABS, &CPU::A);
};
TEST_F(M6502LoadRegTest, LDYAbsLoadValIntoAReg) {
    TestAbsLoadReg(CPU::INS_LDY_ABS, &CPU::Y);
};
TEST_F(M6502LoadRegTest, LDXAbsLoadValIntoAReg) {
    TestAbsLoadReg(CPU::INS_LDX_ABS, &CPU::X);
};
TEST_F(M6502LoadRegTest, LDAAbsXLoadValIntoAReg) {
    TestAbsXLoadReg(CPU::INS_LDA_ABSX, &CPU::A);
};

TEST_F(M6502LoadRegTest, LDYAbsXLoadValIntoAReg) {
    TestAbsXLoadReg(CPU::INS_LDY_ABSX, &CPU::Y);
};
TEST_F(M6502LoadRegTest, LDAAbsXLoadValIntoARegWhenItCrossesAPage) {
    TestAbsXLoadRegWhenItCrossesPage(CPU::INS_LDA_ABSX, &CPU::A);
};
TEST_F(M6502LoadRegTest, LDYAbsXLoadValIntoARegWhenItCrossesAPage) {
    TestAbsXLoadRegWhenItCrossesPage(CPU::INS_LDY_ABSX, &CPU::Y);
};

TEST_F(M6502LoadRegTest, LDAAbsYLoadValIntoARegWhenItCrossesAPage) {
    TestAbsYLoadRegWhenItCrossesPage(CPU::INS_LDA_ABSY, &CPU::A);
};
TEST_F(M6502LoadRegTest, LDYAbsYLoadValIntoARegWhenItCrossesAPage) {
    TestAbsYLoadRegWhenItCrossesPage(CPU::INS_LDX_ABSY, &CPU::X);
};

TEST_F(M6502LoadRegTest, LDAAbsYLoadValIntoAReg) {
	TestAbsYLoadReg(CPU::INS_LDA_ABSY, &CPU::A);
};
TEST_F(M6502LoadRegTest, LDXAbsYLoadValIntoAReg) {
	TestAbsYLoadReg(CPU::INS_LDX_ABSY, &CPU::X);
};

TEST_F(M6502LoadRegTest, LDAIndirectXLoadValIntoAReg) {
    cpu.X = 0x04;
    cpu.Z = cpu.N = true;
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

TEST_F(M6502LoadRegTest, LDAIndirectYLoadValIntoAReg) {
    cpu.Y = 0x04;
    cpu.Z = cpu.N = true;
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
}
TEST_F(M6502LoadRegTest, LDAIndirectYLoadValIntoARegWhenItCrossesAPage) {
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

