#include "gtest/gtest.h"
#include "6502.h"
using namespace m6502;
/**
; TestPrg

* = $1000

lda #$FF

start
sta $90
sta $8000
eor #$CC
jmp start

*/
static m6502::Byte TestPrg[] = {
    0x00, 0x10, 0xA9, 0xFF, 0x85, 0x90,
    0x8D, 0x00, 0x80, 0x49, 0xCC, 0x4C, 0x02, 0x10 };

static const m6502::u32 NumBytesInPrg = 14;
class M6502LoadProgTest : public testing::Test {

public:
    Mem mem;
    CPU cpu;
    virtual void SetUp() {
        cpu.Reset(mem );
    }
    virtual void TearDown() { }
};

TEST_F(M6502LoadProgTest, LoadProg) {
    
    cpu.LoadProg(TestPrg, NumBytesInPrg, mem);

    EXPECT_EQ(mem[0x1000], 0xA9);
    EXPECT_EQ(mem[0x1001], 0xFF);
    EXPECT_EQ(mem[0x1002], 0x85);
}
TEST_F(M6502LoadProgTest, TestLoadProgramAProgramAndExecuteIt)
{
    // given:
    using namespace m6502;

    // when:
    Word StartAddress = cpu.LoadProg(TestPrg, NumBytesInPrg, mem);
    cpu.PC = StartAddress;

    //then:
    for (m6502::s32 Clock = 1000; Clock > 0; )
    {
        Clock -= cpu.Execute(1, mem);
    }
}

TEST_F(M6502LoadProgTest, LoadThe6502TestPrg)
{
// ./as65.exe -l -m -w -h0 6502_functional_test.a65
//#if 0
    // given:
    using namespace m6502;

    // when:
    FILE* fp;
    fopen_s(&fp,
        "6502_functional_test.bin", "rb");

    fread(&mem[0x000A], 1, 65526, fp);
    fclose(fp);

    cpu.PC = 0x400;

    //then:
    while (true)
    {
        cpu.Execute(1, mem);
    }
//#endif
}
