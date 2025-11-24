#include "backend/targets/riscv64/rv64_defs.h"
#include <backend/targets/riscv64/passes/lowering/frame_lowering.h>
#include <debug.h>

namespace BE::RV64::Passes::Lowering
{
    void FrameLoweringPass::runOnModule(BE::Module& module)
    {
        for (auto* func : module.functions) runOnFunction(func);
    }

    void FrameLoweringPass::runOnFunction(BE::Function* func) { TODO("实现 Frame Lowering"); }

}  // namespace BE::RV64::Passes::Lowering
