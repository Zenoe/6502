#include <stdio.h>
#include <stdlib.h>
#include "6502.h"

using namespace std;
using namespace m6502;

int test() {

    Mem mem;
    CPU cpu;
    /*
    cpu.Reset(mem);
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x42;
    cpu.Execute(2, mem);
    */

    cpu.Reset(mem);
    mem[0xFFFC] = CPU::INS_JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::INS_LDA_IM;
    mem[0x4243] = 0x84;
    cpu.Execute(9, mem);

    return 0;
}

//int main(int argc, char* argv[]) {
//    test();
//    return 0;
//}
