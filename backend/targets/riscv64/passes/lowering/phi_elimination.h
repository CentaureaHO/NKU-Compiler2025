#ifndef __BACKEND_RV64_PASSES_LOWERING_PHI_ELIMINATION_H__
#define __BACKEND_RV64_PASSES_LOWERING_PHI_ELIMINATION_H__

#include <backend/mir/m_module.h>
#include <backend/mir/m_function.h>
#include <backend/mir/m_block.h>
#include <backend/mir/m_instruction.h>
#include <backend/common/cfg.h>
#include <backend/common/cfg_builder.h>
#include <backend/target/target_instr_adapter.h>
#include <backend/targets/riscv64/rv64_defs.h>
#include <map>
#include <queue>
#include <vector>

namespace BE::RV64::Passes::Lowering
{
    class PhiEliminationPass
    {
      public:
        PhiEliminationPass()  = default;
        ~PhiEliminationPass() = default;

        void runOnModule(BE::Module& module, const BE::Targeting::TargetInstrAdapter* adapter);

      private:
        void runOnFunction(BE::Function* func, const BE::Targeting::TargetInstrAdapter* adapter);
    };
}  // namespace BE::RV64::Passes::Lowering

#endif  // __BACKEND_RV64_PASSES_LOWERING_PHI_ELIMINATION_H__
