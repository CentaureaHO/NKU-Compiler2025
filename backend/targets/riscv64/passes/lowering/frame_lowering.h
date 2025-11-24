#ifndef __BACKEND_RV64_PASSES_LOWERING_FRAME_LOWERING_H__
#define __BACKEND_RV64_PASSES_LOWERING_FRAME_LOWERING_H__

#include <backend/mir/m_module.h>
#include <backend/mir/m_function.h>
#include <backend/mir/m_block.h>
#include <backend/mir/m_instruction.h>
#include <backend/mir/m_defs.h>
#include <backend/targets/riscv64/rv64_defs.h>
#include <vector>

namespace BE::RV64::Passes::Lowering
{
    class FrameLoweringPass
    {
      public:
        FrameLoweringPass()  = default;
        ~FrameLoweringPass() = default;

        void runOnModule(BE::Module& module);

      private:
        void runOnFunction(BE::Function* func);
    };
}  // namespace BE::RV64::Passes::Lowering

#endif  // __BACKEND_RV64_PASSES_LOWERING_FRAME_LOWERING_H__
