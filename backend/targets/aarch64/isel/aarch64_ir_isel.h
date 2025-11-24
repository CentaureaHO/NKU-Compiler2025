#ifndef __BACKEND_TARGETS_RISCV64_ISEL_RV64_IR_ISEL_H__
#define __BACKEND_TARGETS_RISCV64_ISEL_RV64_IR_ISEL_H__

#include <backend/isel/isel_base.h>

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

/*
 * 这一部分的逻辑单纯的就是遍历 IR，然后根据 IR 生成对应的目标指令
 * 这一部分和从 AST 到 IR 的转换逻辑类似，都是一种“逐节点翻译”的过程
 * 例如，你可能会在 IR 中创建这样的指令：
 *    %a = add i32 %b, %c
 * 那么你只需要通过某种方式记录，目前 %b 和 %c 的结果分别存储在哪个寄存器中，然后生成对应的目标指令即可
 * 例如，%b 目前存储在后端创建的虚拟寄存器 v_1_i32， %c 存储在 v_2_i32，那么你只需要生成类似下面的指令：
 *    add v_3_i32, v_1_i32, v_2_i32
 * 随后记录一下 ，%a 的结果存储在 v_3_i32 中即可，后续如果有指令使用到 %a，那么通过查表的方式使用 v_3_i32 来代表 %a 即可
 *
 * 因此，这里就不给出过多的注释了与解释了，类似工作在生成中间代码的时候已经做过了
 */

namespace BE::Targeting
{
    class BackendTarget;
}

namespace BE::AArch64
{
    class IRIsel : public BE::ISelBase<IRIsel>, public BE::IRIselBase
    {
        friend class BE::ISelBase<IRIsel>;

      public:
        IRIsel(ME::Module* ir_module, BE::Module* backend_module, BE::Targeting::BackendTarget* target)
            : BE::ISelBase<IRIsel>(backend_module), ir_module_(ir_module), target_(target)
        {}

      private:
        ME::Module*                   ir_module_;
        BE::Targeting::BackendTarget* target_;

        void runImpl();

      public:
        void visit(ME::Module& module) override;
        void visit(ME::Function& func) override;
        void visit(ME::Block& block) override;

        void visit(ME::LoadInst& inst) override;
        void visit(ME::StoreInst& inst) override;
        void visit(ME::ArithmeticInst& inst) override;
        void visit(ME::IcmpInst& inst) override;
        void visit(ME::FcmpInst& inst) override;
        void visit(ME::AllocaInst& inst) override;
        void visit(ME::BrCondInst& inst) override;
        void visit(ME::BrUncondInst& inst) override;
        void visit(ME::CallInst& inst) override;
        void visit(ME::RetInst& inst) override;
        void visit(ME::GEPInst& inst) override;
        void visit(ME::FP2SIInst& inst) override;
        void visit(ME::SI2FPInst& inst) override;
        void visit(ME::ZextInst& inst) override;
        void visit(ME::PhiInst& inst) override;

        void visit(ME::GlbVarDeclInst& inst) override;
        void visit(ME::FuncDeclInst& inst) override;
        void visit(ME::FuncDefInst& inst) override;
    };
}  // namespace BE::AArch64

#endif
