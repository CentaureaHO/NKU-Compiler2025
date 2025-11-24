#include <backend/targets/aarch64/passes/lowering/frame_lowering.h>
#include <backend/targets/aarch64/aarch64_defs.h>
#include <backend/mir/m_block.h>
#include <backend/mir/m_function.h>
#include <debug.h>
#include <algorithm>

namespace BE::AArch64::Passes::Lowering
{
    void FrameLoweringPass::runOnModule(BE::Module& module)
    {
        for (auto* func : module.functions)
        {
            if (!func) continue;
            lowerFunction(func);
        }
    }

    void FrameLoweringPass::lowerFunction(BE::Function* func) { TODO("实现 Frame Lowering"); }
}  // namespace BE::AArch64::Passes::Lowering
