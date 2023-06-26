#include "gtest/gtest.h"
#include "6502.h"
using namespace m6502;
class M6502StoreRegTest : public testing::Test {

public:
    Mem mem;
    CPU cpu;
    virtual void SetUp() {
        cpu.Reset(mem );
    }
    virtual void TearDown() {

    }

    void TestStoreRegZP(Byte opCode, Byte CPU::*Reg );
    void TestStoreRegZPX(Byte opCode, Byte CPU::*Reg );
    void TestStoreRegZPY(Byte opCode, Byte CPU::*Reg );
    void TestStoreRegAbs(Byte opCode, Byte CPU::*Reg );
    void TestSTAAbsX(Byte opCode, Byte CPU::*Reg);
    void TestSTAAbsY(Byte opCode, Byte CPU::*Reg);
};

static void VerifyUnmodifiedFlagsFromLDA(const CPU& cpu, const CPU& cpuCopy) {
    EXPECT_EQ(cpu.C, cpuCopy.C);
    EXPECT_EQ(cpu.I, cpuCopy.I);
    EXPECT_EQ(cpu.D, cpuCopy.D);
    EXPECT_EQ(cpu.B, cpuCopy.B);
    EXPECT_EQ(cpu.V, cpuCopy.V);
    EXPECT_EQ(cpu.Z, cpuCopy.Z);
    EXPECT_EQ(cpu.N, cpuCopy.N);
}

void M6502StoreRegTest::TestStoreRegZP(Byte opCode, Byte CPU::*reg) {
    cpu.*reg = 0x2F;
    mem[0xFFFC] = opCode;
    mem[0xFFFD] = 0x80;
    mem[0x0080] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 3;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x0080], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
void M6502StoreRegTest::TestStoreRegZPX(Byte opCode, Byte CPU::*reg) {
    cpu.*reg = 0x2F;
    cpu.X = 0x0F;
    mem[0xFFFC] = opCode;
    mem[0xFFFD] = 0x80;
    mem[0x0080] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x008F], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
void M6502StoreRegTest::TestStoreRegZPY(Byte opCode, Byte CPU::*reg) {
    cpu.*reg = 0x2F;
    cpu.Y = 0x0F;
    mem[0xFFFC] = opCode;
    mem[0xFFFD] = 0x80;
    mem[0x0080] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x008F], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

void M6502StoreRegTest::TestStoreRegAbs(Byte opCode, Byte CPU::*reg) {
    cpu.*reg = 0x2F;
    mem[0xFFFC] = opCode;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x77;
    mem[0x7780] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x7780], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
void M6502StoreRegTest::TestSTAAbsX(Byte opCode, Byte CPU::*reg) {
    cpu.*reg = 0x2F;
    cpu.X = 0x0F;
    mem[0xFFFC] = opCode;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x77;
    mem[0x7780] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x778F], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
void M6502StoreRegTest::TestSTAAbsY(Byte opCode, Byte CPU::*reg) {
    cpu.*reg = 0x2F;
    cpu.Y = 0x0F;
    mem[0xFFFC] = opCode;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x77;
    mem[0x7780] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 4;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x778F], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
TEST_F(M6502StoreRegTest, STA0Page) {
    TestStoreRegZP(CPU::INS_STA_ZP, &CPU::A);
}
//stx
TEST_F(M6502StoreRegTest, STX0Page) {
    TestStoreRegZP(CPU::INS_STX_ZP, &CPU::X);
}
TEST_F(M6502StoreRegTest, STX0PageY) {
    TestStoreRegZPY(CPU::INS_STX_ZPY, &CPU::X);
}
// sty
TEST_F(M6502StoreRegTest, STY0Page) {
    TestStoreRegZP(CPU::INS_STY_ZP, &CPU::Y);
}
TEST_F(M6502StoreRegTest, STY0PageX) {
    TestStoreRegZPX(CPU::INS_STY_ZPX, &CPU::Y);
}

TEST_F(M6502StoreRegTest, STAAbs) {
    TestStoreRegAbs(CPU::INS_STA_ABS, &CPU::A);
}
TEST_F(M6502StoreRegTest, STXAbs) {
    TestStoreRegAbs(CPU::INS_STX_ABS, &CPU::X);
}
TEST_F(M6502StoreRegTest, STYAbs) {
    TestStoreRegAbs(CPU::INS_STY_ABS, &CPU::Y);
}
TEST_F(M6502StoreRegTest, STAAbsX) {
    TestSTAAbsX(CPU::INS_STA_ABSX, &CPU::A);
}
TEST_F(M6502StoreRegTest, STAAbsY) {
    TestSTAAbsY(CPU::INS_STA_ABSY, &CPU::A);
}

TEST_F(M6502StoreRegTest, STAIndX) {
    //Indexed Indirect. normally used in conjunction with a table of address held on zero page.
   //The address of the table is taken from the instruction and the X register added to it (with zero page wrap around) to give the location of the least significant byte of the target address.
    cpu.X = 0x0F;
    cpu.A = 0x2F;
    mem[0xFFFC] = CPU::INS_STA_INDX;
    mem[0xFFFD] = 0x80;
    mem[0x008F] = 0x00;
    mem[0x0090] = 0x33;
    mem[0x3300] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 6;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x3377+0x04], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
TEST_F(M6502StoreRegTest, STAIndY) {
    //Indirect Indexed 
    //In instruction contains the zero page location of the least significant byte of 16 bit address. The Y register is dynamically added to this value to generated the actual target address for operation. 
    cpu.Y = 0x04;
    cpu.A = 0x2F;
    mem[0xFFFC] = CPU::INS_STA_INDY;
    mem[0xFFFD] = 0x80;
    mem[0x0080] = 0x00;
    mem[0x0081] = 0x33;
    mem[0x3300+0x04] = 0x00;
    CPU cpuCopy = cpu;
    constexpr s32 EXPECTCYCLES = 5;
    const s32 actualCycles = cpu.Execute(EXPECTCYCLES, mem);

    EXPECT_EQ(actualCycles, EXPECTCYCLES);
    EXPECT_EQ(mem[0x3377+0x04], 0x2F);
    VerifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}
