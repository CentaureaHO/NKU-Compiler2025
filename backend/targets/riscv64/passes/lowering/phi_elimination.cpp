#include <backend/targets/riscv64/passes/lowering/phi_elimination.h>
#include <debug.h>
#include <algorithm>

namespace BE::RV64::Passes::Lowering
{
    using namespace BE;
    using namespace BE::RV64;

    void PhiEliminationPass::runOnModule(BE::Module& module, const BE::Targeting::TargetInstrAdapter* adapter)
    {
        if (module.functions.empty()) return;
        for (auto* func : module.functions) runOnFunction(func, adapter);
    }

    void PhiEliminationPass::runOnFunction(BE::Function* func, const BE::Targeting::TargetInstrAdapter* adapter)
    {
        TODO("实现 Phi Elimination");
    }
}  // namespace BE::RV64::Passes::Lowering
