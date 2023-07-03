// 6502test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "gtest/gtest.h"
#include "6502.h"
using namespace m6502;
class M6502LogicOperationTest : public testing::Test {
public:
    Mem mem;
    CPU cpu;
    virtual void SetUp() {
        cpu.Reset(mem );
    }
    virtual void TearDown() {

    }
	static void VerifyUnmodifiedFlags(const CPU& cpu, const CPU& cpuCopy) {
		EXPECT_EQ(cpu.Flag.C, cpuCopy.Flag.C);
		EXPECT_EQ(cpu.Flag.I, cpuCopy.Flag.I);
		EXPECT_EQ(cpu.Flag.D, cpuCopy.Flag.D);
		EXPECT_EQ(cpu.Flag.B, cpuCopy.Flag.B);
		EXPECT_EQ(cpu.Flag.V, cpuCopy.Flag.V);
	}
	Byte getIns(ELogicalOp logicalOp, const std::string& type) {
        if (type == "IM") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_IM;
			case ELogicalOp::Or:
				return CPU::INS_ORA_IM;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_IM;
			}
        }
        else if (type == "0Page") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_ZP;
			case ELogicalOp::Or:
				return CPU::INS_ORA_ZP;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_ZP;
			}
        }
        else if (type == "0PageX") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_ZPX;
			case ELogicalOp::Or:
				return CPU::INS_ORA_ZPX;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_ZPX;
			}
        }
        else if (type == "Abs") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_ABS;
			case ELogicalOp::Or:
				return CPU::INS_ORA_ABS;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_ABS;
			}
        }
        else if (type == "AbsX") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_ABSX;
			case ELogicalOp::Or:
				return CPU::INS_ORA_ABSX;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_ABSX;
			}
        }
        else if (type == "AbsY") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_ABSY;
			case ELogicalOp::Or:
				return CPU::INS_ORA_ABSY;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_ABSY;
			}
        }
        else if (type == "IndirectX") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_INDX;
			case ELogicalOp::Or:
				return CPU::INS_ORA_INDX;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_INDX;
			}
        }
        else if (type == "IndirectY") {
			switch (logicalOp)
			{
			case ELogicalOp::And:
				return CPU::INS_AND_INDY;
			case ELogicalOp::Or:
				return CPU::INS_ORA_INDY;
			case ELogicalOp::Eor:
				return CPU::INS_EOR_INDY;
			}
        }
        throw - 1;
    }

    void TestALogicalOpImmediate(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        mem[0xFFFC] = getIns(logicalOp, "IM");
        mem[0xFFFD] = 0x84;
        CPU cpuCopy = cpu;
        s32 cyclesUsed = cpu.Execute(2, mem);
        const Byte expectedResult = DoLogicalOp(0xCC, 0x84, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, 2);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    void TestALogicalOp0Page(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        mem[0xFFFC] = getIns(logicalOp, "0Page");
        mem[0xFFFD] = 0x84;
        mem[0x0084] = 0x37;
        CPU cpuCopy = cpu;
        constexpr Byte EXPECTCYCLES = 3;
        s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);
        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    void TestALogicalOp0PageX(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        cpu.X = 5;
        mem[0xFFFC] = getIns(logicalOp, "0PageX");
        mem[0xFFFD] = 0x84;
        mem[0x0084 + 5] = 0x37;
        CPU cpuCopy = cpu;
        constexpr Byte EXPECTCYCLES = 4;
        s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);
        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    void TestALogicalOp0PagexWhenWrap(ELogicalOp logicalOp) {
        mem[0xFFFC] = getIns(logicalOp, "0PageX");
        cpu.A = 0xCC;
        cpu.X = 0x01;
		mem[0xFFFD] = 0xFF;
		mem[0x0000] = 0x37; // 0xFF+0x01 wraps

		constexpr s32 EXPECTCYCLES = 4;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    void TestALogicalOpAbs(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        mem[0xFFFC] = getIns(logicalOp, "Abs");
		mem[0xFFFD] = 0x80;
		mem[0xFFFE] = 0x44;
		mem[0x4480] = 0x37;
		constexpr s32 EXPECTCYCLES = 4;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    
    void TestALogicalOpAbsX(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        cpu.X = 0x05;
        mem[0xFFFC] = getIns(logicalOp, "AbsX");
		mem[0xFFFD] = 0x80;
		mem[0xFFFE] = 0x44;
		mem[0x4485] = 0x37;
		constexpr s32 EXPECTCYCLES = 4;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    void TestALogicalOpAbsY(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        cpu.Y = 0x05;
        mem[0xFFFC] = getIns(logicalOp, "AbsY");
		mem[0xFFFD] = 0x80;
		mem[0xFFFE] = 0x44;
		mem[0x4485] = 0x37;
		constexpr s32 EXPECTCYCLES = 4;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    void TestALogicalOpAbsXCrossPage(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        cpu.X = 0x01;
        mem[0xFFFC] = getIns(logicalOp, "AbsX");
		mem[0xFFFD] = 0xFF;
		mem[0xFFFE] = 0x44;
		mem[0x4500] = 0x37;
		constexpr s32 EXPECTCYCLES = 5;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
    void TestALogicalOpAbsYCrossPage(ELogicalOp logicalOp) {
        cpu.A = 0xCC;
        cpu.Y = 0x01;
        mem[0xFFFC] = getIns(logicalOp, "AbsY");
		mem[0xFFFD] = 0xFF;
		mem[0xFFFE] = 0x44;
		mem[0x4500] = 0x37;
		constexpr s32 EXPECTCYCLES = 5;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}

    void TestALogicalOpIndirectX(ELogicalOp logicalOp) {
        mem[0xFFFC] = getIns(logicalOp, "IndirectX");
		cpu.X = 0x04;
		cpu.A = 0xCC;
		cpu.Flag.Z = cpu.Flag.N = true;
		mem[0xFFFD] = 0x02;
		mem[0x0006] = 0x00; // 0x04+0x02
		mem[0x0007] = 0x80;
		mem[0x8000] = 0x37;
		constexpr s32 EXPECTCYCLES = 6;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
    }

    void TestALogicalOpIndirectY(ELogicalOp logicalOp) {
        mem[0xFFFC] = getIns(logicalOp, "IndirectY");
		cpu.Y = 0x04;
		cpu.A = 0xCC;
		cpu.Flag.Z = cpu.Flag.N = true;
		mem[0xFFFD] = 0x02;
		mem[0x0002] = 0x00;
		mem[0x0003] = 0x80;
		mem[0x8004] = 0x37;
		constexpr s32 EXPECTCYCLES = 5;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
    }

	void TestALogicalOpIndirectYCrossPage(ELogicalOp logicalOp) {
        mem[0xFFFC] = getIns(logicalOp, "IndirectY");
		cpu.A = 0xCC;
		cpu.Y = 0xFF;
		mem[0xFFFD] = 0x02;
		mem[0x0002] = 0x02;
		mem[0x0003] = 0x80;
		mem[0x8101] = 0x37;
		constexpr s32 EXPECTCYCLES = 6;
		CPU cpuCopy = cpu;
		s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

        const Byte expectedResult = DoLogicalOp(0xCC, 0x37, logicalOp);
        const bool expectedNegative = (expectedResult & 0b10000000) > 0;
		EXPECT_EQ(cpu.A, expectedResult);
		EXPECT_FALSE(cpu.Flag.Z);
		EXPECT_EQ(cpu.Flag.N, expectedNegative);
		EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
		VerifyUnmodifiedFlags(cpu, cpuCopy);
	}
};

TEST_F(M6502LogicOperationTest, TestANDOnARegisterImmediate)
{
    TestALogicalOpImmediate(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterImmediate)
{
    TestALogicalOpImmediate(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterImmediate)
{
    TestALogicalOpImmediate(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegister0Page)
{
    TestALogicalOp0Page(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegister0Page)
{
    TestALogicalOp0Page(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegister0Page)
{
    TestALogicalOp0Page(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegister0PageX)
{
    TestALogicalOp0PageX(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegister0PageX)
{
    TestALogicalOp0PageX(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegister0PageX)
{
    TestALogicalOp0PageX(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegister0PagexWhenWrap)
{
    TestALogicalOp0PagexWhenWrap(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegister0PagexWhenWrap)
{
    TestALogicalOp0PagexWhenWrap(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegister0PagexWhenWrap)
{
    TestALogicalOp0PagexWhenWrap(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegisterAbs)
{
    TestALogicalOpAbs(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterAbs)
{
    TestALogicalOpAbs(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterAbs)
{
    TestALogicalOpAbs(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegisterAbsX)
{
    TestALogicalOpAbsX(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterAbsX)
{
    TestALogicalOpAbsX(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterAbsX)
{
    TestALogicalOpAbsX(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegisterAbsY)
{
    TestALogicalOpAbsY(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterAbsY)
{
    TestALogicalOpAbsY(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterAbsY)
{
    TestALogicalOpAbsY(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegisterAbsXCrossPage)
{
    TestALogicalOpAbsXCrossPage(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterAbsXCrossPage)
{
    TestALogicalOpAbsXCrossPage(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterAbsXCrossPage)
{
    TestALogicalOpAbsXCrossPage(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegisterAbsYCrossPage)
{
    TestALogicalOpAbsYCrossPage(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterAbsYCrossPage)
{
    TestALogicalOpAbsYCrossPage(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterAbsYCrossPage)
{
    TestALogicalOpAbsYCrossPage(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegisterIndirectX)
{
    TestALogicalOpIndirectX(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterIndirectX)
{
    TestALogicalOpIndirectX(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterIndirectX)
{
    TestALogicalOpIndirectX(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestANDOnARegisterIndirectY)
{
    TestALogicalOpIndirectY(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterIndirectY)
{
    TestALogicalOpIndirectY(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterIndirectY)
{
    TestALogicalOpIndirectY(ELogicalOp::Eor);
}


TEST_F(M6502LogicOperationTest, TestANDOnARegisterIndirectYCrossPage)
{
    TestALogicalOpIndirectYCrossPage(ELogicalOp::And);
}
TEST_F(M6502LogicOperationTest, TestOrOnARegisterIndirectYCrossPage)
{
    TestALogicalOpIndirectYCrossPage(ELogicalOp::Or);
}
TEST_F(M6502LogicOperationTest, TestEorOnARegisterIndirectYCrossPage)
{
    TestALogicalOpIndirectYCrossPage(ELogicalOp::Eor);
}

TEST_F(M6502LogicOperationTest, TestBit0Page)
{
	cpu.A = 0xCC;
	mem[0xFFFC] = CPU::INS_BIT_ZP;
	mem[0xFFFD] = 0x42;
	mem[0x0042] = 0b11000000;
	cpu.Flag.Z = true;
	cpu.Flag.V = false;
	cpu.Flag.N = false;
	constexpr s32 EXPECTCYCLES = 3;
	s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_TRUE(cpu.Flag.V);
	EXPECT_TRUE(cpu.Flag.N);
	EXPECT_EQ(cpu.A, 0xCC);
	EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
}

TEST_F(M6502LogicOperationTest, TestBit0PageZeroBits6AndBit7)
{
	cpu.A = 0b00000011;
	mem[0xFFFC] = CPU::INS_BIT_ZP;
	mem[0xFFFD] = 0x42;
	mem[0x0042] = 0b00110000;
	cpu.Flag.Z = true;
	cpu.Flag.V = false;
	cpu.Flag.N = false;
	constexpr s32 EXPECTCYCLES = 3;
	s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

	EXPECT_TRUE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.V);
	EXPECT_FALSE(cpu.Flag.N);
	EXPECT_EQ(cpu.A, 0b00000011);
	EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
}

TEST_F(M6502LogicOperationTest, TestBitAbs)
{
	cpu.A = 0xCC;
	mem[0xFFFC] = CPU::INS_BIT_ABS;
	mem[0xFFFD] = 0x00;
	mem[0xFFFE] = 0x80;
	mem[0x8000] = 0b11000000;
	cpu.Flag.Z = true;
	cpu.Flag.V = false;
	cpu.Flag.N = false;
	constexpr s32 EXPECTCYCLES = 4;
	s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_TRUE(cpu.Flag.V);
	EXPECT_TRUE(cpu.Flag.N);
	EXPECT_EQ(cpu.A, 0xCC);
	EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
}

TEST_F(M6502LogicOperationTest, TestBitAbsZeroBits6AndBit7)
{
	cpu.A = 0xCC;
	mem[0xFFFC] = CPU::INS_BIT_ABS;
	mem[0xFFFD] = 0x00;
	mem[0xFFFE] = 0x80;
	mem[0x8000] = 0b00110011;
	constexpr s32 EXPECTCYCLES = 4;
	s32 cyclesUsed = cpu.Execute(EXPECTCYCLES, mem);

	EXPECT_TRUE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.V);
	EXPECT_FALSE(cpu.Flag.N);
	EXPECT_EQ(cpu.A, 0xCC);
	EXPECT_EQ(cyclesUsed, EXPECTCYCLES);
}
