#ifndef __BACKEND_TARGETS_AARCH64_ISEL_AARCH64_DAG_ISEL_H__
#define __BACKEND_TARGETS_AARCH64_ISEL_AARCH64_DAG_ISEL_H__

#include <backend/isel/isel_base.h>
#include <backend/dag/selection_dag.h>
#include <middleend/module/ir_module.h>
#include <map>
#include <set>

/*
 * 注：当前目录下有 aarch64_dag_isel 与 aarch64_ir_isel 两份实现，它们的功能是一致的，你只需要选择其中一份来完成就行
 *
 * dag_isel 基于选择 DAG 来完成指令选择，你需要实现 DAG 的构建后再生成目标指令，总体难度可能更大，但 dag isel
 * 部分（不含 dag 部分）的代码量会更少一些
 *
 * ir_isel 则是直接基于 IR 来完成指令选择， 你只需要遍历 IR，然后根据 IR
 * 生成目标指令即可，总体难度可能更小一些，但代码量会更多一些
 *
 * 额外的，如果你正确实现了 dag 的构建，那么你可以在其它 Target （如 riscv64）中直接复用构建好的 dag，从而减少重复劳动
 */

namespace BE::Targeting
{
    class BackendTarget;
}

namespace BE::AArch64
{
    class DAGIsel : public BE::ISelBase<DAGIsel>
    {
        friend class BE::ISelBase<DAGIsel>;

      public:
        DAGIsel(ME::Module* ir_module, BE::Module* backend_module, BE::Targeting::BackendTarget* target)
            : BE::ISelBase<DAGIsel>(backend_module), ir_module_(ir_module), target_(target)
        {}

      private:
        ME::Module*                   ir_module_;
        BE::Targeting::BackendTarget* target_;

        /**
         * @brief 每个函数级别的上下文信息
         */
        struct FunctionContext
        {
            BE::Function*              mfunc = nullptr;
            std::map<size_t, Register> vregMap;   ///< IR 寄存器 ID -> 后端虚拟寄存器
            std::map<size_t, int>      allocaFI;  ///< IR alloca 寄存器 ID -> 栈帧索引
        };

        FunctionContext ctx_;

        /**
         * @brief 每个基本块级别的状态（每个块重置）
         */
        std::map<const DAG::SDNode*, Register> nodeToVReg_;  ///< DAG 节点 -> 其结果虚拟寄存器
        std::set<const DAG::SDNode*>           selected_;    ///< 已经选择过的节点集合

        void runImpl();
        void importGlobals();
        void selectFunction(ME::Function* ir_func);
        void collectAllocas(ME::Function* ir_func);
        void setupParameters(ME::Function* ir_func);
        void selectBlock(ME::Block* ir_block, const DAG::SelectionDAG& dag);

        // ==================== 阶段 1：调度（Schedule） ====================

        std::vector<const DAG::SDNode*> scheduleDAG(const DAG::SelectionDAG& dag);
        void                            allocateRegistersForNode(const DAG::SDNode* node);

        // ==================== 阶段 2：选择（Select） ====================

        void     selectNode(const DAG::SDNode* node, BE::Block* m_block);
        Register getOperandReg(const DAG::SDNode* node, BE::Block* m_block);
        Register materializeAddress(const DAG::SDNode* node, BE::Block* m_block);
        bool     selectAddress(const DAG::SDNode* addrNode, const DAG::SDNode*& baseNode, int64_t& offset);
        Register getOrCreateVReg(size_t ir_reg_id, BE::DataType* dt);

        void selectCopy(const DAG::SDNode* node, BE::Block* m_block);
        void selectPhi(const DAG::SDNode* node, BE::Block* m_block);
        void selectConst(const DAG::SDNode* node, BE::Block* m_block);
        void selectBinary(const DAG::SDNode* node, BE::Block* m_block);
        void selectUnary(const DAG::SDNode* node, BE::Block* m_block);
        void selectLoad(const DAG::SDNode* node, BE::Block* m_block);
        void selectStore(const DAG::SDNode* node, BE::Block* m_block);
        void selectICmp(const DAG::SDNode* node, BE::Block* m_block);
        void selectFCmp(const DAG::SDNode* node, BE::Block* m_block);
        void selectBranch(const DAG::SDNode* node, BE::Block* m_block);
        void selectCall(const DAG::SDNode* node, BE::Block* m_block);
        void selectRet(const DAG::SDNode* node, BE::Block* m_block);
        void selectCast(const DAG::SDNode* node, BE::Block* m_block);

        int dataTypeSize(BE::DataType* dt);
    };

}  // namespace BE::AArch64

#endif  // __BACKEND_TARGETS_AARCH64_ISEL_AARCH64_DAG_ISEL_H__
