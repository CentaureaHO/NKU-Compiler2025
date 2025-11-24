#ifndef __BACKEND_DAG_DAG_BUILDER_H__
#define __BACKEND_DAG_DAG_BUILDER_H__

#include <backend/dag/selection_dag.h>
#include <backend/dag/isd.h>
#include <middleend/ir_visitor.h>
#include <middleend/module/ir_module.h>
#include <middleend/module/ir_operand.h>
#include <unordered_map>

namespace BE
{
    namespace DAG
    {
        class DAGBuilder : public ME::Visitor_t<void, SelectionDAG&>
        {
          public:
            void build(ME::Block& block, SelectionDAG& dag)
            {
                reg_value_map_.clear();
                apply(*this, block, dag);
            }

            void visit(ME::Module& module, SelectionDAG& dag) override;
            void visit(ME::Function& func, SelectionDAG& dag) override;
            void visit(ME::Block& block, SelectionDAG& dag) override;

            void visit(ME::LoadInst& inst, SelectionDAG& dag) override;
            void visit(ME::StoreInst& inst, SelectionDAG& dag) override;
            void visit(ME::ArithmeticInst& inst, SelectionDAG& dag) override;
            void visit(ME::IcmpInst& inst, SelectionDAG& dag) override;
            void visit(ME::FcmpInst& inst, SelectionDAG& dag) override;
            void visit(ME::AllocaInst& inst, SelectionDAG& dag) override;
            void visit(ME::BrCondInst& inst, SelectionDAG& dag) override;
            void visit(ME::BrUncondInst& inst, SelectionDAG& dag) override;
            void visit(ME::GlbVarDeclInst& inst, SelectionDAG& dag) override;
            void visit(ME::CallInst& inst, SelectionDAG& dag) override;
            void visit(ME::FuncDeclInst& inst, SelectionDAG& dag) override;
            void visit(ME::FuncDefInst& inst, SelectionDAG& dag) override;
            void visit(ME::RetInst& inst, SelectionDAG& dag) override;
            void visit(ME::GEPInst& inst, SelectionDAG& dag) override;
            void visit(ME::FP2SIInst& inst, SelectionDAG& dag) override;
            void visit(ME::SI2FPInst& inst, SelectionDAG& dag) override;
            void visit(ME::ZextInst& inst, SelectionDAG& dag) override;
            void visit(ME::PhiInst& inst, SelectionDAG& dag) override;

          private:
            /**
             * @brief IR 虚拟寄存器 ID 到 DAG 节点的映射
             *
             * 当某个 IR 指令定义了一个虚拟寄存器时（如 %3 = add %1, %2），
             * 我们将 %3 的 ID 映射到对应的 ADD 节点。
             *
             * 当后续指令使用 %3 时，我们直接从这个映射中查找，返回之前创建的
             * ADD 节点，而不是创建新节点。这实现了：
             */
            std::unordered_map<size_t, SDValue> reg_value_map_;

            /**
             * @brief 当前的内存操作链（Chain）
             *
             * Chain 是 SelectionDAG 中用于表示副作用依赖的特殊边。
             * 所有有副作用的操作（STORE/CALL/LOAD）都必须通过 Chain 连接，
             * 确保它们按照程序语义正确的顺序执行。
             *
             * 工作流程：
             * 1. 初始化为 ENTRY_TOKEN（函数入口的 Chain 根）
             * 2. 遇到 STORE 时：创建 STORE 节点，输入当前 Chain，
             *    然后将 currentChain_ 更新为 STORE 节点的输出 Chain
             * 3. 遇到 LOAD 时：将当前 Chain 作为 LOAD 的输入（确保 LOAD 在之前的 STORE 之后）
             * 4. 遇到 CALL 时：类似 STORE 的处理
             */
            SDValue currentChain_;

            /**
             * @brief 获取 IR 操作数对应的 DAG 节点
             * @param op IR 操作数
             * @param dag 当前的 SelectionDAG
             * @param dtype 期望的数据类型，仅作用于寄存器（或可选的，立即数）
             * @return 对应的 SDValue
             *
             * 功能：
             * 1. 如果操作数是寄存器，从 reg_value_map_ 中查找对应的 DAG 节点
             * 2. 如果操作数是立即数，创建 CONST 节点
             * 3. 如果操作数是标签，创建 LABEL 节点
             */
            SDValue getValue(ME::Operand* op, SelectionDAG& dag, BE::DataType* dtype = nullptr);

            /**
             * @brief 记录 IR 指令的定义结果
             * @param res IR 结果操作数（通常是一个虚拟寄存器）
             * @param val 对应的 DAG 节点
             *
             * 将 IR 指令的结果（如 %3 = add %1, %2 中的 %3）
             * 映射到对应的 DAG 节点（ADD 节点），存入 reg_value_map_。
             */
            void setDef(ME::Operand* res, const SDValue& val);

            BE::DataType* mapType(ME::DataType t);
            uint32_t      mapArithmeticOpcode(ME::Operator op, bool isFloat);
        };

    }  // namespace DAG
}  // namespace BE

#endif  // __BACKEND_DAG_DAG_BUILDER_H__
