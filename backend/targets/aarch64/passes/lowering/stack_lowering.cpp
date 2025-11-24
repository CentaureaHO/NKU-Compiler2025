#include <backend/targets/aarch64/passes/lowering/stack_lowering.h>
#include <backend/targets/aarch64/aarch64_defs.h>
#include <backend/targets/aarch64/aarch64_reg_info.h>
#include <backend/mir/m_block.h>
#include <backend/mir/m_function.h>
#include <backend/mir/m_instruction.h>

namespace BE::AArch64::Passes::Lowering
{
    void StackLoweringPass::runOnModule(BE::Module& module)
    {
        for (auto* func : module.functions)
        {
            if (!func) continue;
            lowerFunction(func);
        }
    }

    void StackLoweringPass::lowerFunction(BE::Function* func) { TODO("实现 Stack Lowering"); }
}  // namespace BE::AArch64::Passes::Lowering
