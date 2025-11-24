#include <backend/targets/aarch64/aarch64_instr_adapter.h>
#include <backend/targets/aarch64/aarch64_defs.h>
#include <debug.h>

namespace BE::Targeting::AArch64
{
    using namespace BE::AArch64;

    bool InstrAdapter::isCall(BE::MInstruction* inst) const { TODO("判断是否为调用指令"); }

    bool InstrAdapter::isReturn(BE::MInstruction* inst) const { TODO("判断是否为返回指令"); }

    bool InstrAdapter::isUncondBranch(BE::MInstruction* inst) const { TODO("判断是否为无条件跳转指令"); }

    bool InstrAdapter::isCondBranch(BE::MInstruction* inst) const { TODO("判断是否为条件跳转指令"); }

    int InstrAdapter::extractBranchTarget(BE::MInstruction* inst) const { TODO("从分支指令中提取目标基本块标签 ID"); }

    void InstrAdapter::enumUses(BE::MInstruction* inst, std::vector<BE::Register>& out) const
    {
        TODO("统计指令使用的寄存器，存入vector out中");
    }

    void InstrAdapter::enumDefs(BE::MInstruction* inst, std::vector<BE::Register>& out) const
    {
        TODO("统计指令定义的寄存器，存入vector out中(可能为空集)");
    }

    static void replaceOne(Operand* op, const BE::Register& from, const BE::Register& to)
    {
        if (auto* regOp = dynamic_cast<RegOperand*>(op))
            if (regOp->reg == from) regOp->reg = to;
    }

    // tip: 1.MemOperand 的 base 和 RegOperand 的 reg 都是寄存器，需要分别处理
    //      2.注意不同指令的寄存器使用方式不同，个数也不相同，需要根据指令类型进行处理
    //      3.定义寄存器可能为空集，需要根据指令类型进行处理
    //      4.可以借助replaceOne函数实现
    void InstrAdapter::replaceUse(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const
    {
        (void)replaceOne;
        TODO("实现 replaceUse，替换指令中的使用寄存器，从from替换为to");
    }

    void InstrAdapter::replaceDef(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const
    {
        (void)replaceOne;
        TODO("实现 replaceDef，替换指令中的定义寄存器，从from替换为to");
    }

    void InstrAdapter::enumPhysRegs(BE::MInstruction* inst, std::vector<BE::Register>& out) const
    {
        TODO("实现 enumPhysRegs");
    }

    void InstrAdapter::insertReloadBefore(
        BE::Block* block, std::deque<BE::MInstruction*>::iterator it, const BE::Register& physReg, int frameIndex) const
    {
        TODO("实现 insertReloadBefore");
    }

    void InstrAdapter::insertSpillAfter(
        BE::Block* block, std::deque<BE::MInstruction*>::iterator it, const BE::Register& physReg, int frameIndex) const
    {
        TODO("实现 insertSpillAfter");
    }
}  // namespace BE::Targeting::AArch64
