#ifndef __BACKEND_TARGETS_AARCH64_AARCH64_REG_INFO_H__
#define __BACKEND_TARGETS_AARCH64_AARCH64_REG_INFO_H__

#include <backend/target/target_reg_info.h>
#include <backend/targets/aarch64/aarch64_defs.h>
#include <map>

namespace BE::Targeting::AArch64
{
    class RegInfo : public TargetRegInfo
    {
      public:
        RegInfo() { TODO("实现 RegInfo 的构造，如有必要"); }

        int spRegId() const override { TODO("实现 spRegId"); }
        int raRegId() const override { TODO("实现 raRegId"); }
        int zeroRegId() const override { TODO("实现 zeroRegId"); }

        const std::vector<int>& intArgRegs() const override { TODO("实现 intArgRegs"); }
        const std::vector<int>& floatArgRegs() const override { TODO("实现 floatArgRegs"); }

        const std::vector<int>& calleeSavedIntRegs() const override { TODO("实现 calleeSavedIntRegs"); }
        const std::vector<int>& calleeSavedFloatRegs() const override { TODO("实现 calleeSavedFloatRegs"); }

        const std::vector<int>& reservedRegs() const override { TODO("实现 reservedRegs"); }

        const std::vector<int>& intRegs() const override { TODO("实现 intRegs"); }
        const std::vector<int>& floatRegs() const override { TODO("实现 floatRegs"); }
    };
}  // namespace BE::Targeting::AArch64

#endif  // __BACKEND_TARGETS_AARCH64_AARCH64_REG_INFO_H__
