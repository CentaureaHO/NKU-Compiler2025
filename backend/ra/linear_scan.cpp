#include <backend/ra/linear_scan.h>
#include <backend/mir/m_function.h>
#include <backend/mir/m_instruction.h>
#include <backend/mir/m_block.h>
#include <backend/mir/m_defs.h>
#include <backend/target/target_reg_info.h>
#include <backend/target/target_instr_adapter.h>
#include <backend/common/cfg.h>
#include <backend/common/cfg_builder.h>
#include <utils/dynamic_bitset.h>
#include <debug.h>

#include <map>
#include <set>
#include <unordered_map>
#include <deque>
#include <algorithm>

namespace BE::RA
{
    /*
     * 线性扫描寄存器分配（Linear Scan）教学版说明
     *
     * 目标：将每个虚拟寄存器（vreg）的活跃区间映射到目标机的物理寄存器或栈槽（溢出）。
     *
     * 核心步骤（整数/浮点分开执行，流程相同）：
     * 1) 指令线性化与编号：为函数内所有指令分配全局顺序号，记录每个基本块的 [start, end) 区间，
     *    同时收集调用点（callPoints），用于偏好分配被调用者保存寄存器（callee-saved）。
     * 2) 构建 USE/DEF：枚举每条指令的使用与定义寄存器，聚合到基本块级的 USE/DEF 集合。
     * 3) 活跃性分析：在 CFG 上迭代 IN/OUT，满足 IN = USE ∪ (OUT − DEF) 直至收敛。
     * 4) 活跃区间构建：按基本块从后向前，根据 IN/OUT 与指令次序，累积每个 vreg 的若干 [start, end) 段并合并。
     * 5) 标记跨调用：若区间与任意调用点重叠（交叉），标记 crossesCall=true，以便后续优先使用被调用者保存寄存器。
     * 6) 线性扫描分配：将区间按起点排序，维护活动集合 active；到达新区间时先移除已过期区间，然后
     *    尝试选择空闲物理寄存器；若无空闲则选择一个区间溢出（常见启发：溢出“结束点更远”的区间）。
     * 7) 重写 MIR：对未分配物理寄存器的 use/def，在指令前/后插入 reload/spill，并用临时物理寄存器替换操作数。
     *
     * 提示：
     * - 通过 TargetInstrAdapter 提供的接口完成目标无关的指令读写。
     * - TargetRegInfo 提供了可分配寄存器集合、被调用者保存寄存器、保留寄存器等信息。
     */
    namespace
    {
        struct Segment
        {
            int start;
            int end;
            Segment(int s = 0, int e = 0) : start(s), end(e) {}
        };
        struct Interval
        {
            BE::Register         vreg;
            std::vector<Segment> segs;
            bool                 crossesCall = false;

            void addSegment(int s, int e)
            {
                if (s >= e) return;
                segs.emplace_back(s, e);
            }
            void merge() { TODO("考虑如何合并区间"); }
        };

        struct IntervalOrder
        {
            bool operator()(const Interval* a, const Interval* b) const { TODO("实现 IntervalOrder 的比较"); }
        };
    }  // namespace

    static std::vector<int> buildAllocatableInt(const BE::Targeting::TargetRegInfo& ri)
    {
        TODO("收集可分配的 GPR 集合");
    }
    static std::vector<int> buildAllocatableFloat(const BE::Targeting::TargetRegInfo& ri)
    {
        TODO("收集可分配的 FPR 集合");
    }

    void LinearScanRA::allocateFunction(BE::Function& func, const BE::Targeting::TargetRegInfo& regInfo)
    {
        ASSERT(BE::Targeting::g_adapter && "TargetInstrAdapter is not set");

        std::map<BE::Block*, std::pair<int, int>>                                   blockRange;
        std::vector<std::pair<BE::Block*, std::deque<BE::MInstruction*>::iterator>> id2iter;
        std::set<int>                                                               callPoints;
        int                                                                         ins_id = 0;
        for (auto& [bid, block] : func.blocks)
        {
            int start = ins_id;
            for (auto it = block->insts.begin(); it != block->insts.end(); ++it, ++ins_id)
            {
                id2iter.emplace_back(block, it);
                if (BE::Targeting::g_adapter->isCall(*it)) callPoints.insert(ins_id);
            }
            blockRange[block] = {start, ins_id};
        }

        std::map<BE::Block*, std::set<BE::Register>> USE, DEF;
        for (auto& [bid, block] : func.blocks)
        {
            std::set<BE::Register> use, def;
            for (auto it = block->insts.begin(); it != block->insts.end(); ++it)
            {
                std::vector<BE::Register> uses, defs;
                BE::Targeting::g_adapter->enumUses(*it, uses);
                BE::Targeting::g_adapter->enumDefs(*it, defs);
                for (auto& d : defs)
                    if (!def.count(d)) def.insert(d);
                for (auto& u : uses)
                    if (!def.count(u)) use.insert(u);
            }
            USE[block] = std::move(use);
            DEF[block] = std::move(def);
        }

        // ============================================================================
        // 构建 CFG 后继关系
        // ============================================================================
        // 作用：搭建活跃性数据流的图结构。
        // 如何做：可直接用 MIR::CFGBuilder 生成 CFG，再转换为 succs 映射。
        BE::MIR::CFG*                                 cfg = nullptr;
        std::map<BE::Block*, std::vector<BE::Block*>> succs;
        TODO("RA: 构建 CFG 并获取后继关系");

        // ============================================================================
        // 活跃性分析（IN/OUT）
        // ============================================================================
        // IN[b] = USE[b] ∪ (OUT[b] − DEF[b])，OUT[b] = ⋃ IN[s]，其中 s ∈ succs[b]
        // 迭代执行上述操作直到不变为止
        std::map<BE::Block*, std::set<BE::Register>> IN, OUT;
        bool                                         changed = true;
        while (changed)
        {
            changed = false;
            for (auto& [bid, block] : func.blocks)
            {
                std::set<BE::Register> newOUT;
                for (auto* s : succs[block])
                {
                    auto it = IN.find(s);
                    if (it != IN.end()) newOUT.insert(it->second.begin(), it->second.end());
                }
                std::set<BE::Register> newIN = USE[block];

                for (auto& r : newOUT)
                    if (!DEF[block].count(r)) newIN.insert(r);

                if (!(newOUT != OUT[block] || newIN != IN[block])) continue;

                OUT[block] = std::move(newOUT);
                IN[block]  = std::move(newIN);
                changed    = true;
            }
        }

        delete cfg;

        // ============================================================================
        // 构建活跃区间（Intervals）
        // ============================================================================
        // 作用：得到每个 vreg 的若干 [start,end) 段并合并（interval.merge()）。
        // 如何做：对每个基本块，反向遍历其指令序列，根据 IN/OUT/uses/defs 更新段的开始/结束。
        TODO("RA: 构建活跃区间");

        // ============================================================================
        // 线性扫描主循环
        // ============================================================================
        // 作用：按区间起点排序；进入新区间前，先从活动集合 active 移除“已结束”的区间；
        // 然后尝试分配空闲物理寄存器；若无可用，执行溢出策略（如“溢出结束点更远”的区间）。
        auto allIntRegs   = buildAllocatableInt(regInfo);
        auto allFloatRegs = buildAllocatableFloat(regInfo);
        TODO("RA: 实现 active 维护、空闲物理寄存器选择/冲突检测、溢出策略与记录（assignedPhys / spillFrameIndex）");

        // ============================================================================
        // 重写 MIR（插入 reload/spill，替换 use/def）
        // ============================================================================
        // 作用：将未分配物理寄存器的 use/def 改写为使用 scratch + FILoad/FIStore（由 Adapter 注入）。
        // 如何做：
        // - 对每条指令枚举 uses：若该 vreg 分配了物理寄存器，则直接替换；
        //   否则在指令前插入 reload 到一个 scratch，然后用 scratch 替换 use。
        // - 对每条指令枚举 defs：若分配了物理寄存器则直接替换；
        //   否则先将 def 写到一个 scratch，再在指令后插入 spill 到对应 FI。
        TODO("RA: 调用 Adapter 的接口改写指令");
    }
}  // namespace BE::RA
