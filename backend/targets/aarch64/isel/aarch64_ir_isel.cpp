#include <backend/targets/aarch64/isel/aarch64_ir_isel.h>

namespace BE::AArch64
{
    void IRIsel::runImpl() { apply(*this, *ir_module_); }

    void IRIsel::visit(ME::Module& module) { TODO("Handle armv8 ir isel for module"); }
    void IRIsel::visit(ME::Function& func) { TODO("Handle armv8 ir isel for function"); }
    void IRIsel::visit(ME::Block& block) { TODO("Handle armv8 ir isel for block"); }

    void IRIsel::visit(ME::LoadInst& inst) { TODO("Handle armv8 ir isel for load instruction"); }
    void IRIsel::visit(ME::StoreInst& inst) { TODO("Handle armv8 ir isel for store instruction"); }
    void IRIsel::visit(ME::ArithmeticInst& inst) { TODO("Handle armv8 ir isel for arithmetic instruction"); }
    void IRIsel::visit(ME::IcmpInst& inst) { TODO("Handle armv8 ir isel for icmp instruction"); }
    void IRIsel::visit(ME::FcmpInst& inst) { TODO("Handle armv8 ir isel for fcmp instruction"); }
    void IRIsel::visit(ME::AllocaInst& inst) { TODO("Handle armv8 ir isel for alloca instruction"); }
    void IRIsel::visit(ME::BrCondInst& inst) { TODO("Handle armv8 ir isel for brcond instruction"); }
    void IRIsel::visit(ME::BrUncondInst& inst) { TODO("Handle armv8 ir isel for bruncond instruction"); }
    void IRIsel::visit(ME::CallInst& inst) { TODO("Handle armv8 ir isel for call instruction"); }
    void IRIsel::visit(ME::RetInst& inst) { TODO("Handle armv8 ir isel for ret instruction"); }
    void IRIsel::visit(ME::GEPInst& inst) { TODO("Handle armv8 ir isel for gep instruction"); }
    void IRIsel::visit(ME::FP2SIInst& inst) { TODO("Handle armv8 ir isel for fp2si instruction"); }
    void IRIsel::visit(ME::SI2FPInst& inst) { TODO("Handle armv8 ir isel for si2fp instruction"); }
    void IRIsel::visit(ME::ZextInst& inst) { TODO("Handle armv8 ir isel for zext instruction"); }
    void IRIsel::visit(ME::PhiInst& inst) { TODO("Handle armv8 ir isel for phi instruction"); }

    void IRIsel::visit(ME::GlbVarDeclInst& inst)
    {
        ERROR("Global variable declarations should not appear in IR during instruction selection.");
    }
    void IRIsel::visit(ME::FuncDeclInst& inst)
    {
        ERROR("Function declarations should not appear in IR during instruction selection.");
    }
    void IRIsel::visit(ME::FuncDefInst& inst)
    {
        ERROR("Function definitions should not appear in IR during instruction selection.");
    }
}  // namespace BE::AArch64
