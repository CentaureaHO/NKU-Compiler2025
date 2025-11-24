#ifndef __BACKEND_AARCH64_PASSES_LOWERING_STACK_LOWERING_H__
#define __BACKEND_AARCH64_PASSES_LOWERING_STACK_LOWERING_H__

#include <backend/mir/m_module.h>

namespace BE::AArch64::Passes::Lowering
{
    class StackLoweringPass
    {
      public:
        StackLoweringPass()  = default;
        ~StackLoweringPass() = default;

        void runOnModule(BE::Module& module);

      private:
        void lowerFunction(BE::Function* func);
    };
}  // namespace BE::AArch64::Passes::Lowering

#endif  // __BACKEND_AARCH64_PASSES_LOWERING_STACK_LOWERING_H__
