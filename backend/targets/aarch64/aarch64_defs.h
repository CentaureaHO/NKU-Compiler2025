#ifndef __BACKEND_TARGETS_AARCH64_AARCH64_DEFS_H__
#define __BACKEND_TARGETS_AARCH64_AARCH64_DEFS_H__

#include <backend/mir/m_defs.h>
#include <backend/mir/m_instruction.h>
#include <array>
#include <vector>
#include <string>

namespace BE::AArch64
{
    constexpr int      A64_REGISTER_ID_SP   = 31;
    constexpr int      A64_REGISTER_ID_XZR  = 32;
    constexpr int      A64_GPR_ARG_COUNT    = 8;
    constexpr int      A64_FPR_ARG_COUNT    = 8;
    constexpr int      A64_CALLEE_INT_FIRST = 19;
    constexpr int      A64_CALLEE_INT_LAST  = 28;
    constexpr int      A64_CALLEE_FP_FIRST  = 8;
    constexpr int      A64_CALLEE_FP_LAST   = 15;
    constexpr int      A64_MAX_GPR_ID       = 30;
    constexpr int      A64_MAX_FPR_ID       = 31;
    constexpr unsigned A64_IMM16_MASK       = 0xFFFFu;
    constexpr int      A64_IMM16_SHIFT      = 16;
    constexpr unsigned A64_IMM12_MASK       = 0xFFFu;
    constexpr unsigned A64_WORD_MASK        = 0xFFFFFFFFu;

    // (NAME, TYPE, ASM)
    // TYPE: R (rd, rs1, rs2), R2 (rd, rs), M (rt, [base, off]), P (pair mem), Z (no operands)
#define A64_INSTS             \
    X(ADD, R, "add")          \
    X(SUB, R, "sub")          \
    X(SDIV, R, "sdiv")        \
    X(MUL, R, "mul")          \
    X(AND, R, "and")          \
    X(ORR, R, "orr")          \
    X(EOR, R, "eor")          \
    X(LSL, R, "lsl")          \
    X(LSR, R, "lsr")          \
    X(ASR, R, "asr")          \
    X(MOVZ, R, "movz")        \
    X(MOVN, R, "movn")        \
    X(MOVK, R, "movk")        \
    X(UXTW, R2, "uxtw")       \
    X(LA, R2, "la")           \
    X(CSET, R2, "cset")       \
    X(STP, P, "stp")          \
    X(LDP, P, "ldp")          \
    X(MOV, R2, "mov")         \
    X(LDR, M, "ldr")          \
    X(STR, M, "str")          \
    X(CMP, R2, "cmp")         \
    X(B, L, "b")              \
    X(BEQ, L, "b.eq")         \
    X(BNE, L, "b.ne")         \
    X(BLT, L, "b.lt")         \
    X(BLE, L, "b.le")         \
    X(BGT, L, "b.gt")         \
    X(BGE, L, "b.ge")         \
    X(BL, SYM, "bl")          \
    X(RET, Z, "ret")          \
    X(FADD, R, "fadd")        \
    X(FSUB, R, "fsub")        \
    X(FMUL, R, "fmul")        \
    X(FDIV, R, "fdiv")        \
    X(FCMP, R2, "fcmp")       \
    X(FMOV, R2, "fmov")       \
    X(SCVTF, R2, "scvtf")     \
    X(FCVTZS, R2, "fcvtzs")   \
    /* pseudo for RA */       \
    X(FILOAD, Z, "fi_load")   \
    X(FISTORE, Z, "fi_store") \
    X(NOP, Z, "nop")

    enum class Operator
    {
#define X(n, t, a) n,
        A64_INSTS
#undef X
    };

    enum class OpType
    {
        R,    // rd, rs1, rs2
        R2,   // rd, rs
        M,    // rt, [base, #off]
        P,    // pair mem (stp/ldp)
        L,    // label only
        SYM,  // symbol only (bl)
        Z     // no operands
    };

    std::string getOpInfoAsm(Operator op);
    OpType      getOpInfoType(Operator op);

    class Instr : public BE::MInstruction
    {
      public:
        Operator              op;
        std::vector<Operand*> operands;

        Operand* fiop;  // FrameIndexOperand for lowering
        bool     use_fiops;

        Instr(Operator o) : BE::MInstruction(BE::InstKind::TARGET), op(o), fiop(nullptr), use_fiops(false) {}

        ~Instr()
        {
            for (auto* operand : operands)
            {
                if (!operand) continue;
                delete operand;
                operand = nullptr;
            }
            if (fiop)
            {
                delete fiop;
                fiop = nullptr;
            }
        }
    };

// #define LOC_STR ("Created at: " + std::string(__FILE__) + ":" + std::to_string(__LINE__))
#define LOC_STR ("")

    inline Instr* createInstr_impl(Operator op, std::string comment = "")
    {
        Instr* i   = new Instr(op);
        i->comment = comment;
        return i;
    }
    inline Instr* createInstr_impl(Operator op, Operand* a1, std::string comment = "")
    {
        auto* i = new Instr(op);
        i->operands.push_back(a1);
        i->comment = comment;
        return i;
    }
    inline Instr* createInstr_impl(Operator op, Operand* a1, Operand* a2, std::string comment = "")
    {
        auto* i = new Instr(op);
        i->operands.push_back(a1);
        i->operands.push_back(a2);
        i->comment = comment;
        return i;
    }
    inline Instr* createInstr_impl(Operator op, Operand* a1, Operand* a2, Operand* a3, std::string comment = "")
    {
        auto* i = new Instr(op);
        i->operands.push_back(a1);
        i->operands.push_back(a2);
        i->operands.push_back(a3);
        i->comment = comment;
        return i;
    }
    inline Instr* createInstr_impl(
        Operator op, Operand* a1, Operand* a2, Operand* a3, Operand* a4, std::string comment = "")
    {
        auto* i = new Instr(op);
        i->operands.push_back(a1);
        i->operands.push_back(a2);
        i->operands.push_back(a3);
        i->operands.push_back(a4);
        i->comment = comment;
        return i;
    }

#define createInstr0(op) createInstr_impl(op, LOC_STR)
#define createInstr1(op, a1) createInstr_impl(op, a1, LOC_STR)
#define createInstr2(op, a1, a2) createInstr_impl(op, a1, a2, LOC_STR)
#define createInstr3(op, a1, a2, a3) createInstr_impl(op, a1, a2, a3, LOC_STR)
#define createInstr4(op, a1, a2, a3, a4) createInstr_impl(op, a1, a2, a3, a4, LOC_STR)

    inline std::string formatPhysReg(const Register& r, DataType* dt)
    {
        if (dt == F32) return "s" + std::to_string(r.rId);
        if (dt == F64) return "d" + std::to_string(r.rId);
        if (dt == I32)
        {
            if (r.rId == A64_REGISTER_ID_XZR) return "wzr";
            return "w" + std::to_string(r.rId);
        }
        if (r.rId == A64_REGISTER_ID_SP) return "sp";
        if (r.rId == A64_REGISTER_ID_XZR) return "xzr";
        return "x" + std::to_string(r.rId);
    }

    inline std::string formatRegister(const Register& r)
    {
        if (r.isVreg) return "v_" + std::to_string(r.rId) + "_" + r.dt->toString();
        auto* dt = r.dt != nullptr ? r.dt : I64;
        return formatPhysReg(r, dt);
    }

    inline std::array<unsigned, 4> decomposeImm64(unsigned long long value)
    {
        std::array<unsigned, 4> segments{};
        for (size_t i = 0; i < segments.size(); ++i)
            segments[i] = static_cast<unsigned>((value >> (i * A64_IMM16_SHIFT)) & A64_IMM16_MASK);
        return segments;
    }

    inline bool fitsUnsignedImm12(int value) { return value >= 0 && static_cast<unsigned>(value) <= A64_IMM12_MASK; }

    inline bool fitsUnsignedScaledOffset(int value, int scale)
    {
        if (scale <= 0) return false;
        if (value < 0) return false;
        if (value % scale != 0) return false;
        return static_cast<unsigned>(value / scale) <= A64_IMM12_MASK;
    }

    class ImmeOperand : public Operand
    {
      public:
        int value;
        ImmeOperand(int val) : Operand(I32, Operand::Type::IMMI32), value(val) {}
    };

    class MemOperand : public Operand
    {
      public:
        Register base;
        int      offset;
        MemOperand(Register b, int o) : Operand(PTR, Operand::Type::REG), base(b), offset(o) {}
    };

    class LabelOperand : public Operand
    {
      public:
        int targetBlockId;
        LabelOperand(int bid) : Operand(nullptr, Operand::Type::IMMI32), targetBlockId(bid) {}
    };

    class SymbolOperand : public Operand
    {
      public:
        std::string name;
        SymbolOperand(const std::string& n) : Operand(PTR, Operand::Type::REG), name(n) {}
    };

    inline Instr* createMove(Operand* dst, Operand* src, const std::string& comment = "")
    {
        if (ImmeOperand* imi = dynamic_cast<ImmeOperand*>(src))
        {
            Instr* inst = new Instr(Operator::MOVZ);
            inst->operands.push_back(dst);
            inst->operands.push_back(new ImmeOperand(imi->value));
            inst->comment = comment;
            return inst;
        }

        Operator movop;

        if (dst->dt->equal(src->dt))
            movop = Operator::MOV;
        else if (dst->dt->equal(I64) && src->dt->equal(I32))
            movop = Operator::UXTW;
        else
            TODO("Unsupported move operand types for %s to %s",
                src->dt ? src->dt->toString().c_str() : "null",
                dst->dt ? dst->dt->toString().c_str() : "null");

        Instr* inst = new Instr(movop);
        inst->operands.push_back(dst);
        inst->operands.push_back(src);
        inst->comment = comment;
        return inst;
    }

    inline Instr* createMove(Operand* dst, int imm, const std::string& comment = "")
    {
        return AArch64::createMove(dst, new ImmeOperand(imm), comment);
    }

    namespace PR
    {
#define A64_X_REGS \
    X(x0, 0)       \
    X(x1, 1)       \
    X(x2, 2)       \
    X(x3, 3)       \
    X(x4, 4)       \
    X(x5, 5)       \
    X(x6, 6)       \
    X(x7, 7)       \
    X(x8, 8)       \
    X(x9, 9)       \
    X(x10, 10)     \
    X(x11, 11)     \
    X(x12, 12)     \
    X(x13, 13)     \
    X(x14, 14)     \
    X(x15, 15)     \
    X(x16, 16)     \
    X(x17, 17)     \
    X(x18, 18)     \
    X(x19, 19)     \
    X(x20, 20)     \
    X(x21, 21)     \
    X(x22, 22)     \
    X(x23, 23)     \
    X(x24, 24)     \
    X(x25, 25)     \
    X(x26, 26)     \
    X(x27, 27)     \
    X(x28, 28)     \
    X(x29, 29)     \
    X(x30, 30)     \
    X(sp, 31)      \
    X(xzr, 32)

#define X(name, id) inline Register name(id, I64, false);
        A64_X_REGS
#undef X

#define A64_W_REGS \
    X(w0, 0)       \
    X(w1, 1)       \
    X(w2, 2)       \
    X(w3, 3)       \
    X(w4, 4)       \
    X(w5, 5)       \
    X(w6, 6)       \
    X(w7, 7)       \
    X(w8, 8)       \
    X(w9, 9)       \
    X(w10, 10)     \
    X(w11, 11)     \
    X(w12, 12)     \
    X(w13, 13)     \
    X(w14, 14)     \
    X(w15, 15)     \
    X(w16, 16)     \
    X(w17, 17)     \
    X(w18, 18)     \
    X(w19, 19)     \
    X(w20, 20)     \
    X(w21, 21)     \
    X(w22, 22)     \
    X(w23, 23)     \
    X(w24, 24)     \
    X(w25, 25)     \
    X(w26, 26)     \
    X(w27, 27)     \
    X(w28, 28)     \
    X(w29, 29)     \
    X(w30, 30)     \
    X(wzr, 32)

#define X(name, id) inline Register name(id, I32, false);
        A64_W_REGS
#undef X

#define A64_D_REGS \
    X(d0, 0)       \
    X(d1, 1)       \
    X(d2, 2)       \
    X(d3, 3)       \
    X(d4, 4)       \
    X(d5, 5)       \
    X(d6, 6)       \
    X(d7, 7)       \
    X(d8, 8)       \
    X(d9, 9)       \
    X(d10, 10)     \
    X(d11, 11)     \
    X(d12, 12)     \
    X(d13, 13)     \
    X(d14, 14)     \
    X(d15, 15)     \
    X(d16, 16)     \
    X(d17, 17)     \
    X(d18, 18)     \
    X(d19, 19)     \
    X(d20, 20)     \
    X(d21, 21)     \
    X(d22, 22)     \
    X(d23, 23)     \
    X(d24, 24)     \
    X(d25, 25)     \
    X(d26, 26)     \
    X(d27, 27)     \
    X(d28, 28)     \
    X(d29, 29)     \
    X(d30, 30)     \
    X(d31, 31)

#define X(name, id) inline Register name(id, F64, false);
        A64_D_REGS
#undef X

#define A64_S_REGS \
    X(s0, 0)       \
    X(s1, 1)       \
    X(s2, 2)       \
    X(s3, 3)       \
    X(s4, 4)       \
    X(s5, 5)       \
    X(s6, 6)       \
    X(s7, 7)       \
    X(s8, 8)       \
    X(s9, 9)       \
    X(s10, 10)     \
    X(s11, 11)     \
    X(s12, 12)     \
    X(s13, 13)     \
    X(s14, 14)     \
    X(s15, 15)     \
    X(s16, 16)     \
    X(s17, 17)     \
    X(s18, 18)     \
    X(s19, 19)     \
    X(s20, 20)     \
    X(s21, 21)     \
    X(s22, 22)     \
    X(s23, 23)     \
    X(s24, 24)     \
    X(s25, 25)     \
    X(s26, 26)     \
    X(s27, 27)     \
    X(s28, 28)     \
    X(s29, 29)     \
    X(s30, 30)     \
    X(s31, 31)

#define X(name, id) inline Register name(id, F32, false);
        A64_S_REGS
#undef X
    }  // namespace PR
}  // namespace BE::AArch64

#endif  // __BACKEND_TARGETS_AARCH64_AARCH64_DEFS_H__
