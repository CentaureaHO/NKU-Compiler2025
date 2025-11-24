#include <backend/targets/aarch64/passes/lowering/phi_elimination.h>
#include <debug.h>
#include <algorithm>
#include <transfer.h>

namespace BE::AArch64::Passes::Lowering
{
    using namespace BE;

    void PhiEliminationPass::runOnModule(BE::Module& module, const BE::Targeting::TargetInstrAdapter* adapter)
    {
        if (module.functions.empty()) return;
        for (auto* func : module.functions) runOnFunction(func, adapter);
    }

    void PhiEliminationPass::runOnFunction(BE::Function* func, const BE::Targeting::TargetInstrAdapter* adapter)
    {
        TODO("(如果你在中端实现了mem2reg)实现 Phi Elimination");
    }
}  // namespace BE::AArch64::Passes::Lowering
