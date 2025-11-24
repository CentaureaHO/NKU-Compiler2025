#ifndef __BACKEND_RV64_PASSES_LOWERING_STACK_LOWERING_H__
#define __BACKEND_RV64_PASSES_LOWERING_STACK_LOWERING_H__

#include <backend/mir/m_function.h>
#include <backend/mir/m_module.h>
#include <vector>
#include <map>

namespace BE::RV64::Passes::Lowering
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

}  // namespace BE::RV64::Passes::Lowering

#endif  // __BACKEND_RV64_PASSES_LOWERING_STACK_LOWERING_H__
