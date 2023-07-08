
#include <gtest/gtest.h>
#include "6502.h"

class M6502IncDecTest : public testing::Test
{
public:
	m6502::Mem mem;
	m6502::CPU cpu;

	virtual void SetUp()
	{
		cpu.Reset(mem);
	}

	virtual void TearDown()
	{
	}

	void ExpectUnaffectedRegisters(m6502::CPU CPUBefore)
	{
		EXPECT_EQ(CPUBefore.Flag.C, cpu.Flag.C);
		EXPECT_EQ(CPUBefore.Flag.I, cpu.Flag.I);
		EXPECT_EQ(CPUBefore.Flag.D, cpu.Flag.D);
		EXPECT_EQ(CPUBefore.Flag.B, cpu.Flag.B);
		EXPECT_EQ(CPUBefore.Flag.V, cpu.Flag.V);
	}
};

TEST_F(M6502IncDecTest, INX0)
{
	using namespace m6502;
	cpu.Reset(0xFF00, mem);
	cpu.X = 0x0;
	cpu.Flag.Z = true;
	cpu.Flag.N = true;
	mem[0xFF00] = CPU::INS_INX;
	constexpr s32 EXPECTED_CYCLES = 2;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(cpu.X, 0x01);
	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}

TEST_F(M6502IncDecTest, INX255)
{
	using namespace m6502;
	cpu.Reset(0xFF00, mem);
	cpu.X = 0xFF;
	cpu.Flag.Z = true;
	cpu.Flag.N = true;
	mem[0xFF00] = CPU::INS_INX;
	constexpr s32 EXPECTED_CYCLES = 2;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(cpu.X, 0x0);
	EXPECT_TRUE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}
TEST_F(M6502IncDecTest, INXNegativeVal)
{
	using namespace m6502;
	cpu.Reset(0xFF00, mem);
	cpu.X = 0b10001000;
	cpu.Flag.Z = true;
	cpu.Flag.N = false;
	mem[0xFF00] = CPU::INS_INX;
	constexpr s32 EXPECTED_CYCLES = 2;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(cpu.X, 0b10001001);
	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_TRUE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}

TEST_F(M6502IncDecTest, DEX)
{
	cpu.Reset(0xFF00, mem);
	cpu.X = 0x0;
	mem[0xFF00] = CPU::INS_DEX;
	constexpr s32 EXPECTED_CYCLES = 2;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(cpu.X, 0xFF);
	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_TRUE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}

TEST_F(M6502IncDecTest, INCZP)
{
	cpu.Reset(0xFF00, mem);
	cpu.X = 0x0;
	mem[0xFF00] = CPU::INS_INC_ZP;
	mem[0xFF01] = 0x42;
	mem[0x0042] = 0x67;
	constexpr s32 EXPECTED_CYCLES = 5;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(mem[0x0042], 0x68);
	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}

TEST_F(M6502IncDecTest, INCZPX)
{
	cpu.Reset(0xFF00, mem);
	cpu.X = 0x02;
	mem[0xFF00] = CPU::INS_INC_ZPX;
	mem[0xFF01] = 0x42;
	mem[0x0044] = 0x67;
	constexpr s32 EXPECTED_CYCLES = 6;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(mem[0x0044], 0x68);
	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}

TEST_F(M6502IncDecTest, INCABS)
{
	cpu.Reset(0xFF00, mem);
	mem[0xFF00] = CPU::INS_INC_ABS;
	mem[0xFF01] = 0x02;
	mem[0xFF02] = 0x80;
	mem[0x8002] = 0x67;
	constexpr s32 EXPECTED_CYCLES = 6;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(mem[0x8002], 0x68);
	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}

TEST_F(M6502IncDecTest, INCABSX)
{
	cpu.Reset(0xFF00, mem);
	cpu.X = 0x02;
	mem[0xFF00] = CPU::INS_INC_ABSX;
	mem[0xFF01] = 0x02;
	mem[0xFF02] = 0x80;
	mem[0x8002+0x02] = 0x67;
	constexpr s32 EXPECTED_CYCLES = 6;
	CPU CPUCopy = cpu;
	const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

	EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
	EXPECT_EQ(mem[0x8002+0x02], 0x68);
	EXPECT_FALSE(cpu.Flag.Z);
	EXPECT_FALSE(cpu.Flag.N);
	ExpectUnaffectedRegisters(CPUCopy);
}

