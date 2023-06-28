#include "gtest/gtest.h"
#include "6502.h"
using namespace m6502;
class M6502JmpNCallTest : public testing::Test {

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

TEST_F(M6502JmpNCallTest, JumpToSubroutineNJumpBack) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_JSR;
    mem[0xFF01] = 0x00;
    mem[0xFF02] = 0x80;
    mem[0x8000] = CPU::INS_RTS;
    mem[0xFF03] = CPU::INS_LDA_IM;
    mem[0xFF04] = 0x42;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 6 + 6 + 2;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.SP, cpuCopy.SP);
}
TEST_F(M6502JmpNCallTest, JSRNotAffectTheProcessorStatus) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_JSR;
    mem[0xFF01] = 0x00;
    mem[0xFF02] = 0x80;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 6;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.PS, cpuCopy.PS);
    EXPECT_NE(cpu.SP, cpuCopy.SP);
    EXPECT_EQ(cpu.PC, 0x8000);
}
TEST_F(M6502JmpNCallTest, JMPAbs) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_JMP_ABS;
    mem[0xFF01] = 0x00;
    mem[0xFF02] = 0x80;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 3;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.PS, cpuCopy.PS);
    EXPECT_EQ(cpu.SP, cpuCopy.SP);
    EXPECT_EQ(cpu.PC, 0x8000);
}
TEST_F(M6502JmpNCallTest, JMPIndirect) {
    cpu.Reset(0xFF00, mem);
    mem[0xFF00] = CPU::INS_JMP_IND;
    mem[0xFF01] = 0x00;
    mem[0xFF02] = 0x80;
    mem[0x8000] = 0x00;
    mem[0x8001] = 0x90;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 5;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(cpu.PS, cpuCopy.PS);
    EXPECT_EQ(cpu.SP, cpuCopy.SP);
    EXPECT_EQ(cpu.PC, 0x9000);
}
