#include "gtest/gtest.h"
#include "6502.h"
using namespace m6502;
class M6502StackOperationTest : public testing::Test {
public:
    Mem mem;
    CPU cpu;
    virtual void SetUp() {
        cpu.Reset(mem );
    }
    virtual void TearDown() {

    }

//    void TestStoreRegZP(Byte opCode, Byte CPU::*Reg );
};

// copies the current contents of the stack register into the X register and sets the zero and negative flag as appropriate
TEST_F(M6502StackOperationTest, TSX) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_TSX;
    cpu.Flag.Z = cpu.Flag.N = true;
    cpu.X = 0x00;
    cpu.SP = 0x01;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 2;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.X, 0x01);
    EXPECT_FALSE(cpu.Flag.Z);
    EXPECT_FALSE(cpu.Flag.N);
}
TEST_F(M6502StackOperationTest, TXS) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_TXS;
    cpu.X = 0x08;
    cpu.SP = 0x01;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 2;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.SP, 0x08);
    EXPECT_EQ(cpu.PS, cpuCopy.PS);
}
// push a copy of accumulator on to the stack
TEST_F(M6502StackOperationTest, PHA) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_PHA;
    cpu.A = 0x42;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 3;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.PS, cpuCopy.PS);
    EXPECT_EQ(cpu.SP, 0xFE);
    EXPECT_EQ(mem[cpu.SPToAddress() + 1], cpu.A);
}
// push a copy of the status flags onto the stack
TEST_F(M6502StackOperationTest, PHP) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_PHP;
    cpu.PS = 0xCC;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 3;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.PS, cpuCopy.PS );
    EXPECT_EQ(mem[cpu.SPToAddress() + 1], 0xCC | CPU::UnusedFlagBit | CPU::BreakFlagBit);
    EXPECT_EQ(cpu.SP, 0xFE);
}
// pull accumulator from stack
TEST_F(M6502StackOperationTest, PLA) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_PLA;
    cpu.A = 0x00;
    cpu.SP = 0xEE;
    mem[0x01EF] = 0x42;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_FALSE(cpu.Flag.Z);
    EXPECT_FALSE(cpu.Flag.N);
    EXPECT_EQ(cpu.SP, 0xEF);
}
TEST_F(M6502StackOperationTest, PLANegative) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_PLA;
    cpu.A = 0x00;
    cpu.SP = 0xEE;
    mem[0x01EF] = 0b10000001;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.A, 0b10000001);
    EXPECT_FALSE(cpu.Flag.Z);
    EXPECT_TRUE(cpu.Flag.N);
}
TEST_F(M6502StackOperationTest, PLP) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_PLP;
    cpu.SP = 0xEE;
    cpu.PS = 0;
    mem[0x01EF] = 0x42;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.PS, 0x42);
    EXPECT_EQ(cpu.SP, 0xEF);
}
