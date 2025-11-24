#ifndef __BACKEND_MIR_M_INSTRUCTION_H__
#define __BACKEND_MIR_M_INSTRUCTION_H__

#include <backend/mir/m_defs.h>
#include <map>

namespace BE
{
    class MInstruction
    {
      public:
        InstKind    kind;
        std::string comment;

        uint32_t id;

      public:
        static void delInst(MInstruction* inst)
        {
            if (!inst) return;
            delete inst;
            inst = nullptr;
        }

      protected:
        MInstruction(InstKind k, const std::string& c = "") : kind(k), comment(c) {}
        virtual ~MInstruction() = default;
    };

    class PseudoInst : public MInstruction
    {
      public:
        PseudoInst(InstKind k, const std::string& c = "") : MInstruction(k, c) {}
    };

    class NopInst : public PseudoInst
    {
      public:
        NopInst(const std::string& c = "") : PseudoInst(InstKind::NOP, c) {}
    };

    class PhiInst : public PseudoInst
    {
      public:
        using labelId = uint32_t;
        using srcOp   = Operand*;
        std::map<labelId, srcOp> incomingVals;
        Register                 resReg;

      public:
        PhiInst(Register res, const std::string& c = "") : PseudoInst(InstKind::PHI, c), resReg(res) {}
    };

    class MoveInst : public PseudoInst
    {
      public:
        Operand* src;
        Operand* dest;

      public:
        MoveInst(Operand* s, Operand* d, const std::string& c = "") : PseudoInst(InstKind::MOVE, c), src(s), dest(d) {}
    };

    class FILoadInst : public PseudoInst
    {
      public:
        Register dest;        // destination physical register
        int      frameIndex;  // index into MFrameInfo's spill slots

      public:
        FILoadInst(Register d, int fi, const std::string& c = "")
            : PseudoInst(InstKind::LSLOT, c), dest(d), frameIndex(fi)
        {}
    };

    class FIStoreInst : public PseudoInst
    {
      public:
        Register src;         // source physical register
        int      frameIndex;  // index into MFrameInfo's spill slots

      public:
        FIStoreInst(Register s, int fi, const std::string& c = "")
            : PseudoInst(InstKind::SSLOT, c), src(s), frameIndex(fi)
        {}
    };

    MoveInst* createMove(Operand* dst, Operand* src, const std::string& c = "");
    MoveInst* createMove(Operand* dst, int imme, const std::string& c = "");
    MoveInst* createMove(Operand* dst, float imme, const std::string& c = "");
}  // namespace BE

#endif  // __BACKEND_MIR_M_INSTRUCTION_H__
