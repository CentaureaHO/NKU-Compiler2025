#include "backend/targets/aarch64/aarch64_defs.h"
#include <backend/targets/aarch64/aarch64_target.h>
#include <backend/target/registry.h>
#include <backend/targets/aarch64/isel/aarch64_dag_isel.h>
#include <backend/targets/aarch64/aarch64_reg_info.h>
#include <backend/targets/aarch64/aarch64_instr_adapter.h>
#include <backend/targets/aarch64/aarch64_codegen.h>
#include <backend/ra/linear_scan.h>
#include <backend/targets/aarch64/passes/lowering/frame_lowering.h>
#include <backend/targets/aarch64/passes/lowering/stack_lowering.h>
#include <backend/targets/aarch64/passes/lowering/phi_elimination.h>

#include <debug.h>

#include <map>
#include <vector>

namespace BE
{
    namespace RA
    {
        void setTargetInstrAdapter(const BE::Targeting::TargetInstrAdapter* adapter);
    }
}  // namespace BE

namespace BE::Targeting::AArch64
{
    namespace
    {
        struct AutoRegister
        {
            AutoRegister()
            {
                BE::Targeting::TargetRegistry::registerTargetFactory("aarch64", []() { return new AArch64Target(); });
                BE::Targeting::TargetRegistry::registerTargetFactory("armv8", []() { return new AArch64Target(); });
            }
        } s_auto_register;
    }  // namespace

    void AArch64Target::runPipeline(ME::Module* ir, BE::Module* backend, std::ostream* out)
    {
        static BE::Targeting::AArch64::InstrAdapter s_adapter;
        static BE::Targeting::AArch64::RegInfo      s_regInfo;
        BE::Targeting::setTargetInstrAdapter(&s_adapter);

        TODO("选择一种 Instruction Selector 实现，并完成指令选择");
        // BE::AArch64::DAGIsel isel(ir, backend, this);
        // BE::AArch64::IRIsel isel(ir, backend, this);
        // isel.run();

        // Pre-RA
        {
            BE::AArch64::Passes::Lowering::FrameLoweringPass fl;
            fl.runOnModule(*backend);

            // 对实现了 mem2reg 优化的同学，还需完成 Phi Elimination
            // BE::AArch64::Passes::Lowering::PhiEliminationPass phiElim;
            // phiElim.runOnModule(*backend, &s_adapter);

            TODO("如有需要，需在寄存器分配前完成其它伪指令的消解，如移动指令的消解");
        }

        // RA
        {
            TODO("使用你实现的寄存器分配器进行寄存器分配");
            // BE::RA::LinearScanRA ra;
            // ra.allocate(*backend, s_regInfo);
        }

        // Post-RA
        {
            BE::AArch64::Passes::Lowering::StackLoweringPass sl;
            sl.runOnModule(*backend);
        }

        BE::AArch64::Codegen codegen(backend, *out);
        codegen.generateAssembly();
    }
}  // namespace BE::Targeting::AArch64
