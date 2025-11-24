#ifndef __BACKEND_A64_PASSES_LOWERING_PHI_ELIMINATION_H__
#define __BACKEND_A64_PASSES_LOWERING_PHI_ELIMINATION_H__

#include <backend/mir/m_module.h>
#include <backend/mir/m_function.h>
#include <backend/mir/m_block.h>
#include <backend/mir/m_instruction.h>
#include <backend/common/cfg.h>
#include <backend/common/cfg_builder.h>
#include <backend/target/target_instr_adapter.h>
#include <backend/targets/aarch64/aarch64_defs.h>
#include <map>
#include <queue>
#include <vector>

namespace BE::AArch64::Passes::Lowering
{
    class PhiEliminationPass
    {
      public:
        PhiEliminationPass()  = default;
        ~PhiEliminationPass() = default;

        void runOnModule(BE::Module& module, const BE::Targeting::TargetInstrAdapter* adapter);

      private:
        void runOnFunction(BE::Function* func, const BE::Targeting::TargetInstrAdapter* adapter);

        void splitCriticalEdgesForBlock(BE::Function* func, BE::MIR::CFG* cfg, uint32_t toLabel);
        void redirectEdgeBranch(BE::Function* func, uint32_t fromLabel, uint32_t oldTo, uint32_t newTo);

        std::deque<BE::MInstruction*>::iterator findInsertPoint(
            BE::Block* block, const BE::Targeting::TargetInstrAdapter* adapter);

        void insertPhiCopiesForPred(BE::Function* func, BE::Block* predBlock,
            const std::vector<std::pair<BE::Register, BE::Operand*>>& copies,
            const BE::Targeting::TargetInstrAdapter*                  adapter);
    };
}  // namespace BE::AArch64::Passes::Lowering

#endif  // __BACKEND_A64_PASSES_LOWERING_PHI_ELIMINATION_H__
