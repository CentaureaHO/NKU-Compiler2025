#include <backend/targets/aarch64/aarch64_defs.h>
#include <backend/mir/m_instruction.h>

namespace BE::AArch64
{
    std::string getOpInfoAsm(Operator op)
    {
        switch (op)
        {
#define X(n, t, a) \
    case Operator::n: return a;
            A64_INSTS
#undef X
            default: return "nop";
        }
    }

    OpType getOpInfoType(Operator op)
    {
        switch (op)
        {
#define X(n, t, a) \
    case Operator::n: return OpType::t;
            A64_INSTS
#undef X
            default: return OpType::Z;
        }
    }
}  // namespace BE::AArch64
