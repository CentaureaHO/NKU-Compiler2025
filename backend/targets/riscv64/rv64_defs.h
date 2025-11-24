#ifndef __BACKEND_RV64_RV64_DEFS_H__
#define __BACKEND_RV64_RV64_DEFS_H__

#include <backend/mir/m_instruction.h>

#define RV64_INST_TYPES           \
    X(R)  /* R rd lhs rhs */      \
    X(I)  /* I rd base imme */    \
    X(S)  /* S val base shift */  \
    X(B)  /* B lhs rhs tar */     \
    X(U)  /* U rd val */          \
    X(J)  /* J rd tar */          \
    X(R2) /* R2 rd rs */          \
    X(R4) /* R4 rd rs1 rs2 rs3 */ \
    X(CALL)

#ifdef RV64_ENABLE_ZBA
#undef RV64_ENABLE_ZBA
#endif
#ifdef RV64_ENABLE_ZBB
#undef RV64_ENABLE_ZBB
#endif
#ifdef RV64_ENABLE_ZICSR
#undef RV64_ENABLE_ZICSR
#endif
#ifdef RV64_ENABLE_ZIFENCEI
#undef RV64_ENABLE_ZIFENCEI
#endif
#ifdef RV64_ENABLE_ZICOND
#undef RV64_ENABLE_ZICOND
#endif

#define RV64_ENABLE_ZBA 0
#define RV64_ENABLE_ZBB 0
#define RV64_ENABLE_ZICSR 0
#define RV64_ENABLE_ZIFENCEI 0
#define RV64_ENABLE_ZICOND 0

// (name, type, _asm, latency)
#define RV64_INSTS_BASE          \
    X(ADD, R, add, 1)            \
    X(ADDW, R, addw, 1)          \
    X(SUB, R, sub, 1)            \
    X(SUBW, R, subw, 1)          \
    X(MUL, R, mul, 3)            \
    X(MULW, R, mulw, 3)          \
    X(DIV, R, div, 30)           \
    X(DIVW, R, divw, 30)         \
    X(FADD_S, R, fadd.s, 5)      \
    X(FSUB_S, R, fsub.s, 5)      \
    X(FMUL_S, R, fmul.s, 5)      \
    X(FDIV_S, R, fdiv.s, 30)     \
    X(REM, R, rem, 30)           \
    X(REMW, R, remw, 30)         \
    X(SLL, R, sll, 1)            \
    X(SRL, R, srl, 1)            \
    X(SRA, R, sra, 1)            \
    X(AND, R, and, 1)            \
    X(OR, R, or, 1)              \
    X(XOR, R, xor, 1)            \
    X(SLT, R, slt, 1)            \
    X(SLTU, R, sltu, 1)          \
    X(FEQ_S, R, feq.s, 4)        \
    X(FLT_S, R, flt.s, 4)        \
    X(FLE_S, R, fle.s, 4)        \
    X(FMIN_S, R, fmin.s, 4)      \
    X(FMAX_S, R, fmax.s, 4)      \
                                 \
    X(ADDI, I, addi, 1)          \
    X(ADDIW, I, addiw, 1)        \
    X(SLLI, I, slli, 1)          \
    X(SRLI, I, srli, 1)          \
    X(SRAI, I, srai, 1)          \
    X(SLLIW, I, slliw, 1)        \
    X(SRLIW, I, srliw, 1)        \
    X(SRAIW, I, sraiw, 1)        \
    X(ANDI, I, andi, 1)          \
    X(ORI, I, ori, 1)            \
    X(XORI, I, xori, 1)          \
    X(SLTI, I, slti, 1)          \
    X(SLTIU, I, sltiu, 1)        \
    X(JALR, I, jalr, 1)          \
    X(RET, I, ret, 1)            \
    X(LW, I, lw, 3)              \
    X(LD, I, ld, 3)              \
    X(FLW, I, flw, 2)            \
    X(FLD, I, fld, 2)            \
                                 \
    X(LI, U, li, 1)              \
    X(LUI, U, lui, 1)            \
    X(LA, U, la, 1)              \
                                 \
    X(SW, S, sw, 1)              \
    X(SD, S, sd, 1)              \
    X(FSW, S, fsw, 4)            \
    X(FSD, S, fsd, 4)            \
                                 \
    X(BEQ, B, beq, 1)            \
    X(BNE, B, bne, 1)            \
    X(BLT, B, blt, 1)            \
    X(BGE, B, bge, 1)            \
    X(BLTU, B, bltu, 1)          \
    X(BGEU, B, bgeu, 1)          \
    X(BGT, B, bgt, 1)            \
    X(BLE, B, ble, 1)            \
    X(BGTU, B, bgtu, 1)          \
    X(BLEU, B, bleu, 1)          \
                                 \
    X(JAL, J, jal, 1)            \
                                 \
    X(FMV_W_X, R2, fmv.w.x, 2)   \
    X(FMV_X_W, R2, fmv.x.w, 1)   \
    X(FCVT_S_W, R2, fcvt.s.w, 2) \
    X(FCVT_W_S, R2, fcvt.w.s, 4) \
    X(FMV_S, R2, fmv.s, 2)       \
    X(FMV_D, R2, fmv.d, 2)       \
    X(ZEXT_W, R2, zext.w, 1)     \
    X(FNEG_S, R2, fneg.s, 2)     \
                                 \
    X(FMADD_S, R4, fmadd.s, 5)   \
    X(FMSUB_S, R4, fmsub.s, 5)   \
    X(FNMADD_S, R4, fnmadd.s, 5) \
    X(FNMSUB_S, R4, fnmsub.s, 5) \
                                 \
    X(CALL, CALL, call, 1)

#if RV64_ENABLE_ZBA
#define RV64_INSTS_ZBA           \
    X(SH1ADD, R, sh1add, 1)      \
    X(SH2ADD, R, sh2add, 1)      \
    X(SH3ADD, R, sh3add, 1)      \
    X(SH1ADDUW, R, sh1add.uw, 1) \
    X(SH2ADDUW, R, sh2add.uw, 1) \
    X(SH3ADDUW, R, sh3add.uw, 1)
#else
#define RV64_INSTS_ZBA
#endif

#if RV64_ENABLE_ZBB
#define RV64_INSTS_ZBB  \
    X(MIN, R, min, 1)   \
    X(MAX, R, max, 1)   \
    X(MINU, R, minu, 1) \
    X(MAXU, R, maxu, 1)
#else
#define RV64_INSTS_ZBB
#endif

#if RV64_ENABLE_ZICOND
#define RV64_INSTS_ZICOND         \
    X(CZERO_EQZ, R, czero.eqz, 1) \
    X(CZERO_NEZ, R, czero.nez, 1)
#else
#define RV64_INSTS_ZICOND
#endif

#define RV64_INSTS  \
    RV64_INSTS_BASE \
    RV64_INSTS_ZBA  \
    RV64_INSTS_ZBB  \
    RV64_INSTS_ZICOND

// (name, alias, saver)
// saver: 0: caller-saved, 1: callee-saved, 2: other
#define RV64_REGS         \
    X(x0, x0, 2)          \
    X(x1, ra, 0)          \
    X(x2, sp, 2)          \
    X(x3, gp, 2)          \
    X(x4, tp, 2)          \
    X(x5, t0, 0)          \
    X(x6, t1, 0)          \
    X(x7, t2, 0)          \
    X(x8, fp, 1) /* s0 */ \
    X(x9, s1, 1)          \
    X(x10, a0, 0)         \
    X(x11, a1, 0)         \
    X(x12, a2, 0)         \
    X(x13, a3, 0)         \
    X(x14, a4, 0)         \
    X(x15, a5, 0)         \
    X(x16, a6, 0)         \
    X(x17, a7, 0)         \
    X(x18, s2, 1)         \
    X(x19, s3, 1)         \
    X(x20, s4, 1)         \
    X(x21, s5, 1)         \
    X(x22, s6, 1)         \
    X(x23, s7, 1)         \
    X(x24, s8, 1)         \
    X(x25, s9, 1)         \
    X(x26, s10, 1)        \
    X(x27, s11, 1)        \
    X(x28, t3, 0)         \
    X(x29, t4, 0)         \
    X(x30, t5, 0)         \
    X(x31, t6, 0)         \
    X(f0, ft0, 0)         \
    X(f1, ft1, 0)         \
    X(f2, ft2, 0)         \
    X(f3, ft3, 0)         \
    X(f4, ft4, 0)         \
    X(f5, ft5, 0)         \
    X(f6, ft6, 0)         \
    X(f7, ft7, 0)         \
    X(f8, fs0, 1)         \
    X(f9, fs1, 1)         \
    X(f10, fa0, 0)        \
    X(f11, fa1, 0)        \
    X(f12, fa2, 0)        \
    X(f13, fa3, 0)        \
    X(f14, fa4, 0)        \
    X(f15, fa5, 0)        \
    X(f16, fa6, 0)        \
    X(f17, fa7, 0)        \
    X(f18, fs2, 1)        \
    X(f19, fs3, 1)        \
    X(f20, fs4, 1)        \
    X(f21, fs5, 1)        \
    X(f22, fs6, 1)        \
    X(f23, fs7, 1)        \
    X(f24, fs8, 1)        \
    X(f25, fs9, 1)        \
    X(f26, fs10, 1)       \
    X(f27, fs11, 1)       \
    X(f28, ft8, 0)        \
    X(f29, ft9, 0)        \
    X(f30, ft10, 0)       \
    X(f31, ft11, 0)

namespace BE::RV64
{
    namespace PR
    {
        enum class Reg
        {
#define X(name, alias, saver) name,
            RV64_REGS
#undef X
        };

#define X(name, alias, saver) extern Register alias;
        RV64_REGS
#undef X

        BE::DataType* getPRType(uint32_t rID);
        Register      getPR(uint32_t rID);
    }  // namespace PR

    enum class Operator
    {
#define X(name, type, _asm, latency) name,
        RV64_INSTS
#undef X
    };

    enum class OpType
    {
#define X(t) t,
        RV64_INST_TYPES
#undef X
    };

    struct OpInfo
    {
        std::string _asm;
        OpType type;
        int    latency;

        OpInfo();
        OpInfo(std::string a, OpType t, int lat = 1);
    };

    class Label
    {
      public:
        std::string name;  // data, symbol name, use la
        uint32_t    lnum;  // not data, block id
        bool        is_data;
        bool        is_hi;      // for hi/lo addressing
        int         jmp_label;  // jump target label
        int         seq_label;  // sequential label
        bool        is_la;      // load address

        Label(bool la = false);
        Label(std::string n, bool hi, bool la = false);
        Label(int jmp, int seq, bool la = false);
        Label(int jmp, bool la = false);
    };

    class Instr : public BE::MInstruction
    {
      public:
        Operator    op;
        Register    rd, rs1, rs2;
        int         imme;
        Label       label;
        bool        use_label;
        int         call_ireg_cnt, call_freg_cnt;
        std::string func_name;  // For call instructions
        int         ins_id;     // Instruction ID for register allocation

        Operand* fiop;     // FrameIndexOperand for lowering
        bool     use_ops;  // Whether ops array is used instead of imme/label

        Instr()
            : BE::MInstruction(BE::InstKind::TARGET),
              rd(0, BE::I64, false),
              rs1(0, BE::I64, false),
              rs2(0, BE::I64, false),
              imme(0),
              label(),
              use_label(false),
              call_ireg_cnt(0),
              call_freg_cnt(0),
              func_name(""),
              ins_id(0),
              fiop(nullptr),
              use_ops(false)
        {}
    };

    Instr* createRInst_impl(Operator op, Register rd, Register rs1, Register rs2, const std::string& comment = "");
    Instr* createR2Inst_impl(Operator op, Register rd, Register rs, const std::string& comment = "");

    Instr* createIInst_impl(Operator op, Register rd, Register rs1, int imme, const std::string& comment = "");
    Instr* createIInst_impl(Operator op, Register rd, Register rs1, Label label, const std::string& comment = "");
    Instr* createIInst_impl(Operator op, Register rd, Register rs1, Operand* op3, const std::string& comment = "");

    Instr* createSInst_impl(Operator op, Register val, Register ptr, int imme, const std::string& comment = "");
    Instr* createSInst_impl(Operator op, Register val, Register ptr, Label label, const std::string& comment = "");

    Instr* createBInst_impl(Operator op, Register rs1, Register rs2, Label label, const std::string& comment = "");

    Instr* createUInst_impl(Operator op, Register rd, int imme, const std::string& comment = "");
    Instr* createUInst_impl(Operator op, Register rd, Label label, const std::string& comment = "");

    Instr* createJInst_impl(Operator op, Register rd, Label label, const std::string& comment = "");

    Instr* createCallInst_impl(
        Operator op, std::string name, int ireg_para_cnt, int freg_para_cnt, const std::string& comment = "");

#define LOC_STR ("Created at: " + std::string(__FILE__) + ":" + std::to_string(__LINE__))

    // #define CREATE_WITH_LOC

#ifdef CREATE_WITH_LOC
#define createRInst(op, rd, rs1, rs2) createRInst_impl(op, rd, rs1, rs2, LOC_STR)
#define createR2Inst(op, rd, rs) createR2Inst_impl(op, rd, rs, LOC_STR)
#define createIInst(op, rd, rs1, arg3) createIInst_impl(op, rd, rs1, arg3, LOC_STR)
#define createSInst(op, val, ptr, arg3) createSInst_impl(op, val, ptr, arg3, LOC_STR)
#define createBInst(op, rs1, rs2, label) createBInst_impl(op, rs1, rs2, label, LOC_STR)
#define createUInst(op, rd, arg2) createUInst_impl(op, rd, arg2, LOC_STR)
#define createJInst(op, rd, label) createJInst_impl(op, rd, label, LOC_STR)
#define createCallInst(op, name, ireg, freg) createCallInst_impl(op, name, ireg, freg, LOC_STR)
#else
#define createRInst(op, rd, rs1, rs2) createRInst_impl(op, rd, rs1, rs2)
#define createR2Inst(op, rd, rs) createR2Inst_impl(op, rd, rs)
#define createIInst(op, rd, rs1, arg3) createIInst_impl(op, rd, rs1, arg3)
#define createSInst(op, val, ptr, arg3) createSInst_impl(op, val, ptr, arg3)
#define createBInst(op, rs1, rs2, label) createBInst_impl(op, rs1, rs2, label)
#define createUInst(op, rd, arg2) createUInst_impl(op, rd, arg2)
#define createJInst(op, rd, label) createJInst_impl(op, rd, label)
#define createCallInst(op, name, ireg, freg) createCallInst_impl(op, name, ireg, freg)
#endif
}  // namespace BE::RV64

#endif  // __BACKEND_RV64_RV64_DEFS_H__
