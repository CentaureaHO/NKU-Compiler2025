#include <backend/targets/riscv64/passes/lowering/stack_lowering.h>
#include <backend/mir/m_function.h>
#include <backend/mir/m_instruction.h>
#include <backend/mir/m_defs.h>
#include <backend/targets/riscv64/rv64_defs.h>
#include <algorithm>
#include <backend/targets/riscv64/rv64_reg_info.h>

namespace BE::RV64::Passes::Lowering
{
    void StackLoweringPass::runOnModule(BE::Module& module)
    {
        for (auto* func : module.functions) lowerFunction(func);
    }

    void StackLoweringPass::lowerFunction(BE::Function* func) { TODO("实现 Stack Lowering"); }
}  // namespace BE::RV64::Passes::Lowering
