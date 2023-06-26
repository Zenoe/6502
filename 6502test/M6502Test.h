#pragma once
#include "gtest/gtest.h"
#include "6502.h"
using namespace m6502;
class M6502LoadRegTest : public testing::Test {

public:
    Mem mem;
    CPU cpu;
    virtual void SetUp() {
        cpu.Reset(mem );
    }
    virtual void TearDown() {

    }
    void TestLoadRegImmediate(Byte opCode, Byte CPU::*Reg );
    void TestLoadReg0Page(Byte opCode, Byte CPU::*Reg );
    void TestLoadReg0PageX(Byte opcodeToTest, Byte m6502::CPU::* RegisterToTest);
    void TestLoadReg0PageY(Byte opcodeToTest, Byte m6502::CPU::* RegisterToTest);
    void TestAbsLoadReg(Byte opcodeToTest, Byte m6502::CPU::* RegisterToTest);
    void TestAbsXLoadReg(Byte opcodeToTest, Byte m6502::CPU::* RegisterToTest);
    void TestAbsYLoadReg(Byte opcodeToTest, Byte m6502::CPU::* regToTest);
    void TestAbsXLoadRegWhenItCrossesPage(Byte opcodeToTest, Byte m6502::CPU::* regToTest);
    void TestAbsYLoadRegWhenItCrossesPage(Byte opcodeToTest, Byte m6502::CPU::* regToTest);

};
