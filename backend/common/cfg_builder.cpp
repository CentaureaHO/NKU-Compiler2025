#include <backend/common/cfg_builder.h>

namespace BE::MIR
{
    CFG* CFGBuilder::buildCFGForFunction(BE::Function* func)
    {
        if (func == nullptr || func->blocks.empty()) return nullptr;
        CFG* cfg = new CFG();
        for (auto& [id, block] : func->blocks) cfg->addNewBlock(id, block);
        if (func->blocks.find(0) != func->blocks.end()) cfg->entry_block = func->blocks[0];
        for (auto& [id, block] : func->blocks)
        {
            auto targets = getBlockTargets(block);
            for (auto t : targets)
                if (cfg->blocks.count(t)) cfg->makeEdge(block->blockId, t);
        }
        addFallthroughEdges(func, cfg);
        for (auto& [id, block] : func->blocks)
        {
            if (block->insts.empty()) continue;
            auto* last = block->insts.back();
            if (adapter_->isReturn(last))
            {
                cfg->ret_block = block;
                break;
            }
        }
        return cfg;
    }

    std::vector<uint32_t> CFGBuilder::getBlockTargets(BE::Block* block)
    {
        std::vector<uint32_t> targets;
        if (block->insts.empty()) return targets;
        for (auto* inst : block->insts)
        {
            if (adapter_->isReturn(inst)) return targets;
            if (adapter_->isUncondBranch(inst) || adapter_->isCondBranch(inst))
            {
                int t = adapter_->extractBranchTarget(inst);
                if (t >= 0) targets.push_back(static_cast<uint32_t>(t));
            }
        }
        return targets;
    }

    void CFGBuilder::addFallthroughEdges(BE::Function* func, CFG* cfg)
    {
        for (auto& [id, block] : func->blocks)
        {
            if (block->insts.empty()) continue;
            bool need_fallthrough = true;
            for (auto* inst : block->insts)
            {
                if (adapter_->isReturn(inst))
                {
                    need_fallthrough = false;
                    break;
                }
                if (adapter_->isUncondBranch(inst))
                {
                    need_fallthrough = false;
                    break;
                }
            }
            if (need_fallthrough)
            {
                uint32_t next_id = id + 1;
                if (func->blocks.count(next_id)) cfg->makeEdge(id, next_id);
            }
        }
    }
}  // namespace BE::MIR
