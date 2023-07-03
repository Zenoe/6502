#include "Util.h"

Byte DoLogicalOp(Byte A, Byte B, ELogicalOp LogicalOp)
{
	switch (LogicalOp)
	{
	case ELogicalOp::And:
		return A & B;
		break;
	case ELogicalOp::Or:
		return A | B;
		break;
	case ELogicalOp::Eor:
		return A ^ B;
		break;
	default:
		throw - 1;
		break;
	}
}
