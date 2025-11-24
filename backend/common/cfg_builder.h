#ifndef __BACKEND_COMMON_CFG_BUILDER_H__
#define __BACKEND_COMMON_CFG_BUILDER_H__

#include <backend/common/cfg.h>
#include <backend/mir/m_function.h>
#include <backend/target/target_instr_adapter.h>

namespace BE::MIR
{
    class CFGBuilder
    {
      public:
        explicit CFGBuilder(const BE::Targeting::TargetInstrAdapter* adapter) : adapter_(adapter) {}

        CFG* buildCFGForFunction(BE::Function* func);

      private:
        const BE::Targeting::TargetInstrAdapter* adapter_;
        std::vector<uint32_t>                    getBlockTargets(BE::Block* block);
        void                                     addFallthroughEdges(BE::Function* func, CFG* cfg);
    };
}  // namespace BE::MIR

#endif  // __BACKEND_COMMON_CFG_BUILDER_H__
