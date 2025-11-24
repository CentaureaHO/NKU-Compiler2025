#ifndef __BACKEND_TARGETS_AARCH64_AARCH64_TARGET_H__
#define __BACKEND_TARGETS_AARCH64_AARCH64_TARGET_H__

#include <backend/target/target.h>

namespace BE
{
    class Module;
}
namespace ME
{
    class Module;
}

namespace BE::Targeting::AArch64
{
    class AArch64Target : public BackendTarget
    {
      public:
        const char* getName() const override { return "aarch64"; }
        void        runPipeline(ME::Module* ir, BE::Module* backend, std::ostream* out) override;
    };
}  // namespace BE::Targeting::AArch64

#endif  // __BACKEND_TARGETS_AARCH64_AARCH64_TARGET_H__
