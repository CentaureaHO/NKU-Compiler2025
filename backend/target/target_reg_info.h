#ifndef __BACKEND_TARGET_TARGET_REG_INFO_H__
#define __BACKEND_TARGET_TARGET_REG_INFO_H__

#include <debug.h>

#include <vector>
#include <set>

// 目标寄存器信息（TargetRegInfo）
// 提供目标机物理寄存器的集合与调用约定信息，供寄存器分配、栈帧布局等流程查询。
// 注意：所有寄存器均以“物理寄存器 ID（int）”表示，需要与 BE::Register::rId 对应。

namespace BE
{
    struct DataType;
}

namespace BE::Targeting
{
    class TargetRegInfo
    {
      public:
        virtual ~TargetRegInfo() = default;

        // 栈指针（SP）寄存器 ID
        virtual int spRegId() const { ERROR("Using base target register info spRegId method is not allowed"); }
        // 返回地址（RA/链接寄存器）寄存器 ID
        virtual int raRegId() const { ERROR("Using base target register info raRegId method is not allowed"); }
        // 零寄存器 ID（若目标无零寄存器，可返回一个永远在 reservedRegs() 中的值）
        virtual int zeroRegId() const { ERROR("Using base target register info zeroRegId method is not allowed"); }

        // 整数参数寄存器列表（调用约定从低位到高位的顺序）
        virtual const std::vector<int>& intArgRegs() const
        {
            ERROR("Using base target register info intArgRegs method is not allowed");
        }
        // 浮点参数寄存器列表（调用约定从低位到高位的顺序）
        virtual const std::vector<int>& floatArgRegs() const
        {
            ERROR("Using base target register info floatArgRegs method is not allowed");
        }

        // 被调用者保存（callee-saved）的 GPR 列表
        virtual const std::vector<int>& calleeSavedIntRegs() const
        {
            ERROR("Using base target register info calleeSavedIntRegs method is not allowed");
        }
        // 被调用者保存（callee-saved）的 FPR 列表
        virtual const std::vector<int>& calleeSavedFloatRegs() const
        {
            ERROR("Using base target register info calleeSavedFloatRegs method is not allowed");
        }

        // 保留的物理寄存器集合（不可用于分配），如 sp/zero/平台保留寄存器等
        virtual const std::vector<int>& reservedRegs() const
        {
            ERROR("Using base target register info reservedRegs method is not allowed");
        }

        // 该目标“全部”的 GPR 列表（包含参数/保存/临时等）
        virtual const std::vector<int>& intRegs() const
        {
            ERROR("Using base target register info intRegs method is not allowed");
        }
        // 该目标“全部”的 FPR 列表（包含参数/保存/临时等）
        virtual const std::vector<int>& floatRegs() const
        {
            ERROR("Using base target register info floatRegs method is not allowed");
        }
    };
}  // namespace BE::Targeting

#endif  // __BACKEND_TARGET_TARGET_REG_INFO_H__
