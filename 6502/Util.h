#pragma once
namespace m6502 {
	using Byte = unsigned char;
	using Word = unsigned short;
	using s8 = char;
	using u32 = unsigned int;
	using s32 = signed int;
    struct Mem;
    struct CPU;
    struct StatusFlags;
}

using namespace m6502;

enum class ELogicalOp
{
	And, Eor, Or
};

Byte DoLogicalOp(Byte A, Byte B, ELogicalOp LogicalOp);
