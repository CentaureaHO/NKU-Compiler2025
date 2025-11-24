#include <backend/mir/m_defs.h>
#include <backend/mir/m_instruction.h>
#include <interfaces/middleend/ir_defs.h>
#include <debug.h>

namespace BE
{
    DataType  I32INSTANCE(DataType::Type::INT, DataType::Length::B32);
    DataType  I64INSTANCE(DataType::Type::INT, DataType::Length::B64);
    DataType  F32INSTANCE(DataType::Type::FLOAT, DataType::Length::B32);
    DataType  F64INSTANCE(DataType::Type::FLOAT, DataType::Length::B64);
    DataType  PTRINSTANCE(DataType::Type::INT, DataType::Length::B64);      // Pointer as 64-bit integer
    DataType  TOKENINSTANCE(DataType::Type::TOKEN, DataType::Length::B64);  // Token for chain dependencies
    DataType* I32   = &I32INSTANCE;
    DataType* I64   = &I64INSTANCE;
    DataType* F32   = &F32INSTANCE;
    DataType* F64   = &F64INSTANCE;
    DataType* PTR   = &PTRINSTANCE;
    DataType* TOKEN = &TOKENINSTANCE;

    Register getVReg(DataType* dt)
    {
        static uint32_t vreg_count = 0;
        return Register(vreg_count++, dt, true);
    }

    MoveInst* createMove(Operand* dst, Operand* src, const std::string& c) { return new MoveInst(src, dst, c); }

    MoveInst* createMove(Operand* dst, int imme, const std::string& c)
    {
        return new MoveInst(new I32Operand(imme), dst, c);
    }

    MoveInst* createMove(Operand* dst, float imme, const std::string& c)
    {
        return new MoveInst(new F32Operand(imme), dst, c);
    }

    bool Register::operator<(Register other) const
    {
        if (isVreg != other.isVreg) return isVreg < other.isVreg;
        if (rId != other.rId) return rId < other.rId;
        if (dt != other.dt)
        {
            if (dt->dt != other.dt->dt) return dt->dt < other.dt->dt;
            return dt->dl < other.dt->dl;
        }
        return false;
    }

    bool Register::operator==(Register other) const
    {
        return rId == other.rId && dt == other.dt && isVreg == other.isVreg;
    }
}  // namespace BE
