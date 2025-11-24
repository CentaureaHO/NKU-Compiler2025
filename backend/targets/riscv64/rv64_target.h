#ifndef __BACKEND_TARGETS_RISCV64_RV64_TARGET_H__
#define __BACKEND_TARGETS_RISCV64_RV64_TARGET_H__

#include <backend/target/target.h>

namespace BE
{
    class Module;
}
namespace ME
{
    class Module;
}

namespace BE::Targeting::RV64
{
    class Target : public BackendTarget
    {
      public:
        const char* getName() const override { return "riscv64"; }
        void        runPipeline(ME::Module* ir, BE::Module* backend, std::ostream* out) override;
    };
}  // namespace BE::Targeting::RV64

#endif  // __BACKEND_TARGETS_RISCV64_RV64_TARGET_H__
