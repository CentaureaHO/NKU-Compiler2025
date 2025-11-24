#include <backend/targets/riscv64/rv64_target.h>
#include <backend/target/registry.h>

#include <backend/targets/riscv64/isel/rv64_dag_isel.h>
#include <backend/targets/riscv64/passes/lowering/frame_lowering.h>
#include <backend/targets/riscv64/passes/lowering/stack_lowering.h>
#include <backend/targets/riscv64/passes/lowering/phi_elimination.h>
#include <backend/targets/riscv64/rv64_codegen.h>

#include <backend/common/cfg_builder.h>
#include <backend/ra/linear_scan.h>
#include <backend/targets/riscv64/rv64_reg_info.h>
#include <backend/targets/riscv64/rv64_instr_adapter.h>
#include <backend/dag/dag_builder.h>
#include <backend/targets/riscv64/dag/rv64_dag_legalize.h>

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

namespace BE::Targeting::RV64
{
    namespace
    {
        struct AutoRegister
        {
            AutoRegister()
            {
                BE::Targeting::TargetRegistry::registerTargetFactory("riscv64", []() { return new Target(); });
                BE::Targeting::TargetRegistry::registerTargetFactory("riscv", []() { return new Target(); });
                BE::Targeting::TargetRegistry::registerTargetFactory("rv64", []() { return new Target(); });
            }
        } s_auto_register;
    }  // namespace

    namespace
    {
        static void runPreRAPasses(BE::Module& m, const BE::Targeting::TargetInstrAdapter* adapter)
        {
            BE::RV64::Passes::Lowering::FrameLoweringPass frameLowering;
            frameLowering.runOnModule(m);

            // 对实现了 mem2reg 优化的同学，还需完成 Phi Elimination
            // BE::RV64::Passes::Lowering::PhiEliminationPass phiElim;
            // phiElim.runOnModule(m, adapter);

            TODO("如有需要，需在寄存器分配前完成其它伪指令的消解，如移动指令的消解");
        }
        static void runRAPipeline(BE::Module& m, const BE::Targeting::RV64::RegInfo& regInfo)
        {
            TODO("使用你实现的寄存器分配器进行寄存器分配");
            // BE::RA::LinearScanRA ls;
            // ls.allocate(m, regInfo);
        }
        static void runPostRAPasses(BE::Module& m)
        {
            BE::RV64::Passes::Lowering::StackLoweringPass stackLowering;
            stackLowering.runOnModule(m);
        }
    }  // namespace

    void Target::runPipeline(ME::Module* ir, BE::Module* backend, std::ostream* out)
    {
        static BE::Targeting::RV64::InstrAdapter s_adapter;
        static BE::Targeting::RV64::RegInfo      s_regInfo;
        BE::Targeting::setTargetInstrAdapter(&s_adapter);

        TODO("选择一种 Instruction Selector 实现，并完成指令选择");
        // BE::RV64::DAGIsel isel(ir, backend, this);
        // BE::RV64::IRIsel isel(ir, backend, this);
        // isel.run();

        runPreRAPasses(*backend, &s_adapter);
        runRAPipeline(*backend, s_regInfo);
        runPostRAPasses(*backend);

        BE::RV64::CodeGen codegen(backend, *out);
        codegen.generateAssembly();
    }
}  // namespace BE::Targeting::RV64
