#ifndef __BACKEND_TARGETS_AARCH64_AARCH64_INSTR_ADAPTER_H__
#define __BACKEND_TARGETS_AARCH64_AARCH64_INSTR_ADAPTER_H__

#include <backend/target/target_instr_adapter.h>

namespace BE::Targeting::AArch64
{
    class InstrAdapter : public TargetInstrAdapter
    {
      public:
        bool isCall(BE::MInstruction* inst) const override;
        bool isReturn(BE::MInstruction* inst) const override;
        bool isUncondBranch(BE::MInstruction* inst) const override;
        bool isCondBranch(BE::MInstruction* inst) const override;
        int  extractBranchTarget(BE::MInstruction* inst) const override;
        void enumUses(BE::MInstruction* inst, std::vector<BE::Register>& out) const override;
        void enumDefs(BE::MInstruction* inst, std::vector<BE::Register>& out) const override;
        void replaceUse(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const override;
        void replaceDef(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const override;
        void enumPhysRegs(BE::MInstruction* inst, std::vector<BE::Register>& out) const override;
        void insertReloadBefore(BE::Block* block, std::deque<BE::MInstruction*>::iterator it,
            const BE::Register& physReg, int frameIndex) const override;
        void insertSpillAfter(BE::Block* block, std::deque<BE::MInstruction*>::iterator it, const BE::Register& physReg,
            int frameIndex) const override;
    };
}  // namespace BE::Targeting::AArch64

#endif  // __BACKEND_TARGETS_AARCH64_AARCH64_INSTR_ADAPTER_H__
