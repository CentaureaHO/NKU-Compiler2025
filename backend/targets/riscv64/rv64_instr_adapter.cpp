#include <backend/targets/riscv64/rv64_instr_adapter.h>
#include <backend/targets/riscv64/rv64_defs.h>
#include <algorithm>

namespace BE::Targeting::RV64
{
    using namespace BE::RV64;

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

    static void replaceReg(BE::Register& slot, const BE::Register& from, const BE::Register& to)
    {
        if (slot == from) slot = to;
    }

    void InstrAdapter::replaceUse(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const
    {
        auto* ri = dynamic_cast<Instr*>(inst);
        if (!ri) return;
        replaceReg(ri->rs1, from, to);
        replaceReg(ri->rs2, from, to);
    }

    void InstrAdapter::replaceDef(BE::MInstruction* inst, const BE::Register& from, const BE::Register& to) const
    {
        auto* ri = dynamic_cast<Instr*>(inst);
        if (!ri) return;
        replaceReg(ri->rd, from, to);
    }

    void InstrAdapter::enumPhysRegs(BE::MInstruction* inst, std::vector<BE::Register>& out) const
    {
        TODO("统计该指令中出现的“物理寄存器，存入vector out中");
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
}  // namespace BE::Targeting::RV64
