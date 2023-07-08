#include <gtest/gtest.h>
#include "6502.h"

class M6502AddSubWithCarryTests : public testing::Test
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
		EXPECT_EQ(CPUBefore.Flag.I, cpu.Flag.I);
		EXPECT_EQ(CPUBefore.Flag.D, cpu.Flag.D);
		EXPECT_EQ(CPUBefore.Flag.B, cpu.Flag.B);
	}

	struct ADCTestData
	{
		bool Carry;
		m6502::Byte A;
		m6502::Byte Operand;
		m6502::Byte Answer;

		bool ExpectC;
		bool ExpectZ;
		bool ExpectN;
		bool ExpectV;
	};

	enum class EOperation
	{
		Add, Subtract
	};

	void TestSBCAbsolute(ADCTestData Test)
	{
		TestADCAbsolute(Test, EOperation::Subtract);
	}

	void TestADCAbsolute(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC_ABS : CPU::INS_SBC_ABS;
		mem[0xFF01] = 0x00;
		mem[0xFF02] = 0x80;
		mem[0x8000] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 4;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}

	void TestSBCAbsoluteX(ADCTestData Test)
	{
		TestADCAbsoluteX(Test, EOperation::Subtract);
	}

	void TestADCAbsoluteX(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.X = 0x10;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC_ABSX : CPU::INS_SBC_ABSX;
		mem[0xFF01] = 0x00;
		mem[0xFF02] = 0x80;
		mem[0x8000 + 0x10] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 4;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}

	void TestSBCAbsoluteY(ADCTestData Test)
	{
		TestADCAbsoluteY(Test, EOperation::Subtract);
	}

	void TestADCAbsoluteY(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.Y = 0x10;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC_ABSY : CPU::INS_SBC_ABSY;
		mem[0xFF01] = 0x00;
		mem[0xFF02] = 0x80;
		mem[0x8000 + 0x10] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 4;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}

	void TestSBCImmediate(ADCTestData Test)
	{
		TestADCImmediate(Test, EOperation::Subtract);
	}

	void TestADCImmediate(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC : CPU::INS_SBC;
		mem[0xFF01] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 2;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}

	void TestSBCZeroPage(ADCTestData Test)
	{
		TestADCZeroPage(Test, EOperation::Subtract);
	}

	void TestADCZeroPage(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC_ZP : CPU::INS_SBC_ZP;
		mem[0xFF01] = 0x42;
		mem[0x0042] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 3;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}

	void TestSBCZeroPageX(ADCTestData Test)
	{
		TestADCZeroPageX(Test, EOperation::Subtract);
	}

	void TestADCZeroPageX(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.X = 0x10;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC_ZPX : CPU::INS_SBC_ZPX;
		mem[0xFF01] = 0x42;
		mem[0x0042 + 0x10] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 4;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}

	void TestSBCIndirectX(ADCTestData Test)
	{
		TestADCIndirectX(Test, EOperation::Subtract);
	}

	void TestADCIndirectX(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.X = 0x04;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC_INDX : CPU::INS_SBC_INDX;
		mem[0xFF01] = 0x02;
		mem[0x0006] = 0x00;	//0x2 + 0x4
		mem[0x0007] = 0x80;
		mem[0x8000] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 6;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}

	void TestSBCIndirectY(ADCTestData Test)
	{
		TestADCIndirectY(Test, EOperation::Subtract);
	}

	void TestADCIndirectY(ADCTestData Test,
		EOperation Operation = EOperation::Add)
	{
		// given:
		using namespace m6502;
		cpu.Reset(0xFF00, mem);
		cpu.Flag.C = Test.Carry;
		cpu.Y = 0x04;
		cpu.A = Test.A;
		cpu.Flag.Z = !Test.ExpectZ;
		cpu.Flag.N = !Test.ExpectN;
		cpu.Flag.V = !Test.ExpectV;
		mem[0xFF00] =
			(Operation == EOperation::Add) ?
			CPU::INS_ADC_INDY : CPU::INS_SBC_INDY;
		mem[0xFF01] = 0x02;
		mem[0x0002] = 0x00;
		mem[0x0003] = 0x80;
		mem[0x8000 + 0x04] = Test.Operand;
		constexpr s32 EXPECTED_CYCLES = 5;
		CPU CPUCopy = cpu;

		// when:
		const s32 ActualCycles = cpu.Execute(EXPECTED_CYCLES, mem);

		// then:
		EXPECT_EQ(ActualCycles, EXPECTED_CYCLES);
		EXPECT_EQ(cpu.A, Test.Answer);
		EXPECT_EQ(cpu.Flag.C, Test.ExpectC);
		EXPECT_EQ(cpu.Flag.Z, Test.ExpectZ);
		EXPECT_EQ(cpu.Flag.N, Test.ExpectN);
		EXPECT_EQ(cpu.Flag.V, Test.ExpectV);
		ExpectUnaffectedRegisters(CPUCopy);
	}
};

#define BYTE( A ) ( (m6502::Byte)A )


TEST_F(M6502AddSubWithCarryTests, ADCImmediateCanAddTwoUnsignedNumbers)
{
	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = 17;
	Test.Answer = 38;
	Test.ExpectC = false;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCImmediate(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCImmediateCanAddAPositiveAndNegativeNumber)
{
	// A: 00010100 20
	// O: 11101111 -17
	// =: 00000011
	// C:1 N:0 V:0 Z:0

	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = BYTE(-17);
	Test.Answer = 4;
	Test.ExpectC = true;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCImmediate(Test);
}
TEST_F(M6502AddSubWithCarryTests, ADCZeroPageCanAddTwoUnsignedNumbers)
{
	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = 17;
	Test.Answer = 38;
	Test.ExpectC = false;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCZeroPage(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCZeroPageCanAddAPositiveAndNegativeNumber)
{
	// A: 00010100 20
	// O: 11101111 -17
	// =: 00000011
	// C:1 N:0 V:0 Z:0

	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = BYTE(-17);
	Test.Answer = 4;
	Test.ExpectC = true;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCZeroPage(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCZeroPageXCanAddTwoUnsignedNumbers)
{
	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = 17;
	Test.Answer = 38;
	Test.ExpectC = false;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCZeroPageX(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCZeroPageXCanAddAPositiveAndNegativeNumber)
{
	// A: 00010100 20
	// O: 11101111 -17
	// =: 00000011
	// C:1 N:0 V:0 Z:0

	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = BYTE(-17);
	Test.Answer = 4;
	Test.ExpectC = true;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCZeroPageX(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCAbsXCanAddTwoUnsignedNumbers)
{
	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = 17;
	Test.Answer = 38;
	Test.ExpectC = false;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCAbsoluteX(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCAbsXCanAddAPositiveAndNegativeNumber)
{
	// A: 00010100 20
	// O: 11101111 -17
	// =: 00000011
	// C:1 N:0 V:0 Z:0

	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = BYTE(-17);
	Test.Answer = 4;
	Test.ExpectC = true;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCAbsoluteX(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCAbsYCanAddTwoUnsignedNumbers)
{
	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = 17;
	Test.Answer = 38;
	Test.ExpectC = false;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCAbsoluteY(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCAbsYCanAddAPositiveAndNegativeNumber)
{
	// A: 00010100 20
	// O: 11101111 -17
	// =: 00000011
	// C:1 N:0 V:0 Z:0

	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = BYTE(-17);
	Test.Answer = 4;
	Test.ExpectC = true;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCAbsoluteY(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCIndXCanAddTwoUnsignedNumbers)
{
	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = 17;
	Test.Answer = 38;
	Test.ExpectC = false;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCIndirectX(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCIndXCanAddAPositiveAndNegativeNumber)
{
	// A: 00010100 20
	// O: 11101111 -17
	// =: 00000011
	// C:1 N:0 V:0 Z:0

	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = BYTE(-17);
	Test.Answer = 4;
	Test.ExpectC = true;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCIndirectX(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCIndYCanAddTwoUnsignedNumbers)
{
	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = 17;
	Test.Answer = 38;
	Test.ExpectC = false;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCIndirectY(Test);
}

TEST_F(M6502AddSubWithCarryTests, ADCIndYCanAddAPositiveAndNegativeNumber)
{
	// A: 00010100 20
	// O: 11101111 -17
	// =: 00000011
	// C:1 N:0 V:0 Z:0

	ADCTestData Test;
	Test.Carry = true;
	Test.A = 20;
	Test.Operand = BYTE(-17);
	Test.Answer = 4;
	Test.ExpectC = true;
	Test.ExpectN = false;
	Test.ExpectV = false;
	Test.ExpectZ = false;
	TestADCIndirectY(Test);
}

