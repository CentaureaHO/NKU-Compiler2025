#include <backend/targets/riscv64/rv64_defs.h>
#include <debug.h>

namespace BE::RV64::PR
{
    BE::DataType* getPRType(uint32_t rID)
    {
        ASSERT(rID < 64 && "RV64 phyreg id out of range");
        if (rID < 32) return BE::I64;
        return BE::F64;
    }
    Register getPR(uint32_t rID)
    {
        DataType* dt = getPRType(rID);
        return Register(rID, dt, false);
    }

#define X(name, alias, saver) Register alias = getPR(static_cast<uint32_t>(Reg::name));
    RV64_REGS
#undef X
}  // namespace BE::RV64::PR

namespace BE::RV64
{
    Label::Label(bool la) : lnum(0), is_data(false), is_hi(false), jmp_label(-1), seq_label(-1), is_la(la) {}

    Label::Label(std::string n, bool hi, bool la)
        : name(n), lnum(0), is_data(true), is_hi(hi), jmp_label(-1), seq_label(-1), is_la(la)
    {}

    Label::Label(int jmp, int seq, bool la)
        : lnum(0), is_data(false), is_hi(false), jmp_label(jmp), seq_label(seq), is_la(la)
    {}

    Label::Label(int jmp, bool la) : lnum(jmp), is_data(false), is_hi(false), jmp_label(jmp), seq_label(-1), is_la(la)
    {}

    OpInfo::OpInfo() {}
    OpInfo::OpInfo(std::string a, OpType t, int lat) : _asm(a), type(t), latency(lat) {}

    Instr* createRInst_impl(Operator op, Register rd, Register rs1, Register rs2, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rd      = rd;
        inst->rs1     = rs1;
        inst->rs2     = rs2;
        inst->comment = comment;
        return inst;
    }

    Instr* createR2Inst_impl(Operator op, Register rd, Register rs, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rd      = rd;
        inst->rs1     = rs;
        inst->comment = comment;
        return inst;
    }

    Instr* createIInst_impl(Operator op, Register rd, Register rs1, int imme, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rd      = rd;
        inst->rs1     = rs1;
        inst->imme    = imme;
        inst->comment = comment;
        return inst;
    }

    Instr* createIInst_impl(Operator op, Register rd, Register rs1, Label /* label */, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rd      = rd;
        inst->rs1     = rs1;
        inst->comment = comment;
        // TODO: Handle label for instruction
        return inst;
    }

    Instr* createIInst_impl(Operator op, Register rd, Register rs1, Operand* op3, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rd      = rd;
        inst->rs1     = rs1;
        inst->fiop    = op3;  // Third operand (for I-type: imme/label/frameindex)
        inst->use_ops = true;
        inst->comment = comment;
        return inst;
    }

    Instr* createSInst_impl(Operator op, Register val, Register ptr, int imme, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rs1     = val;
        inst->rs2     = ptr;
        inst->imme    = imme;
        inst->comment = comment;
        return inst;
    }

    Instr* createSInst_impl(Operator op, Register val, Register ptr, Label /* label */, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rs1     = val;
        inst->rs2     = ptr;
        inst->comment = comment;
        // TODO: Handle label for instruction
        return inst;
    }

    Instr* createBInst_impl(Operator op, Register rs1, Register rs2, Label label, const std::string& comment)
    {
        Instr* inst     = new Instr();
        inst->op        = op;
        inst->rs1       = rs1;
        inst->rs2       = rs2;
        inst->label     = label;
        inst->use_label = true;
        inst->comment   = comment;
        return inst;
    }

    Instr* createUInst_impl(Operator op, Register rd, int imme, const std::string& comment)
    {
        Instr* inst   = new Instr();
        inst->op      = op;
        inst->rd      = rd;
        inst->imme    = imme;
        inst->comment = comment;
        return inst;
    }

    Instr* createUInst_impl(Operator op, Register rd, Label label, const std::string& comment)
    {
        Instr* inst     = new Instr();
        inst->op        = op;
        inst->rd        = rd;
        inst->label     = label;
        inst->use_label = true;
        inst->comment   = comment;
        return inst;
    }

    Instr* createJInst_impl(Operator op, Register rd, Label label, const std::string& comment)
    {
        Instr* inst     = new Instr();
        inst->op        = op;
        inst->rd        = rd;
        inst->label     = label;
        inst->use_label = true;
        inst->comment   = comment;
        return inst;
    }

    Instr* createCallInst_impl(
        Operator op, std::string name, int ireg_para_cnt, int freg_para_cnt, const std::string& comment)
    {
        Instr* inst         = new Instr();
        inst->op            = op;
        inst->func_name     = name;
        inst->call_ireg_cnt = ireg_para_cnt;
        inst->call_freg_cnt = freg_para_cnt;
        inst->comment       = comment;
        return inst;
    }
}  // namespace BE::RV64
