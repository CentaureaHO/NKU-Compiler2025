#ifndef __BACKEND_TARGET_TARGET_INSTR_ADAPTER_H__
#define __BACKEND_TARGET_TARGET_INSTR_ADAPTER_H__

#include <backend/mir/m_block.h>
#include <backend/mir/m_instruction.h>
#include <backend/mir/m_defs.h>
#include <debug.h>
#include <deque>
#include <vector>

/*
 * TargetInstrAdapter
 *
 * 作用：
 * - 为“目标相关”的 MIR 指令提供统一的语义查询与改写接口。
 * - 通用后端流程（如 CFG 构建、寄存器分配、溢出/回填插入）通过该接口而不直接依赖具体 ISA。
 */
namespace BE::Targeting
{
    class TargetInstrAdapter
    {
      public:
        virtual ~TargetInstrAdapter() = default;

        // 是否为“调用”指令（会触发调用约定：参数、返回值、被调用者保存寄存器等）
        virtual bool isCall(BE::MInstruction* inst) const
        {
            ERROR("Using base target instruction adapter isCall method is not allowed");
        }
        // 是否为“返回”指令（函数出口）
        virtual bool isReturn(BE::MInstruction* inst) const
        {
            ERROR("Using base target instruction adapter isReturn method is not allowed");
        }
        // 是否为“无条件跳转”指令
        virtual bool isUncondBranch(BE::MInstruction* inst) const
        {
            ERROR("Using base target instruction adapter isUncondBranch method is not allowed");
        }
        // 是否为“条件跳转”指令
        virtual bool isCondBranch(BE::MInstruction* inst) const
        {
            ERROR("Using base target instruction adapter isCondBranch method is not allowed");
        }
        // 从（无/有条件）分支指令中提取目标基本块标签 ID，如果 inst 不是分支或格式不匹配，返回 -1
        virtual int extractBranchTarget(BE::MInstruction* inst) const
        {
            ERROR("Using base target instruction adapter extractBranchTarget method is not allowed");
        }

        // 枚举“使用（读）”寄存器：包括显式与必要的隐式使用
        virtual void enumUses(BE::MInstruction* inst, std::vector<BE::Register>& out) const
        {
            ERROR("Using base target instruction adapter enumUses method is not allowed");
        }
        // 枚举“定义（写）”寄存器：包括显式与必要的隐式定义
        virtual void enumDefs(BE::MInstruction* inst, std::vector<BE::Register>& out) const
        {
            ERROR("Using base target instruction adapter enumDefs method is not allowed");
        }

        // 将 inst 中的某个“使用”寄存器 from 替换为 to
        virtual void replaceUse(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const
        {
            ERROR("Using base target instruction adapter replaceUse method is not allowed");
        }
        // 将 inst 中的某个“定义”寄存器 from 替换为 to
        virtual void replaceDef(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const
        {
            ERROR("Using base target instruction adapter replaceDef method is not allowed");
        }

        // 枚举该指令中出现的“物理寄存器”（显式+隐式），用于避免与固定寄存器冲突
        virtual void enumPhysRegs(BE::MInstruction* inst, std::vector<BE::Register>& out) const
        {
            ERROR("Using base target instruction adapter enumPhysRegs method is not allowed");
        }

        // 在 it 指向的指令“之前”插入：从帧槽 frameIndex 读取到 physReg 的回填（reload）
        // 由 RA 在遇到溢出的 use 时调用
        virtual void insertReloadBefore(BE::Block* block, std::deque<BE::MInstruction*>::iterator it,
            const BE::Register& physReg, int frameIndex) const
        {
            ERROR("Using base target instruction adapter insertReloadBefore method is not allowed");
        }

        // 在 it 指向的指令“之后”插入：将 physReg 写回帧槽 frameIndex 的溢出（spill）
        // 由 RA 在遇到溢出的 def 时调用
        virtual void insertSpillAfter(BE::Block* block, std::deque<BE::MInstruction*>::iterator it,
            const BE::Register& physReg, int frameIndex) const
        {
            ERROR("Using base target instruction adapter insertSpillAfter method is not allowed");
        }
    };

    inline const TargetInstrAdapter* g_adapter = nullptr;
    inline void                      setTargetInstrAdapter(const TargetInstrAdapter* adapter) { g_adapter = adapter; }
}  // namespace BE::Targeting

#endif  // __BACKEND_TARGET_TARGET_INSTR_ADAPTER_H__
