#include <backend/targets/aarch64/aarch64_codegen.h>
#include <backend/targets/aarch64/aarch64_defs.h>
#include <sstream>

namespace BE::AArch64
{
    static std::string formatOperand(Operand* op)
    {
        if (auto* regOp = dynamic_cast<RegOperand*>(op)) return formatRegister(regOp->reg);
        if (auto* immOp = dynamic_cast<ImmeOperand*>(op)) return "#" + std::to_string(immOp->value);
        if (auto* memOp = dynamic_cast<MemOperand*>(op))
        {
            std::string baseStr = formatRegister(memOp->base);
            return "[" + baseStr + ", #" + std::to_string(memOp->offset) + "]";
        }
        return "unknown_op";
    }

    void Codegen::generateAssembly()
    {
        out_ << ".text\n";
        out_ << ".arch armv8-a\n";

        for (auto* func : module_->functions) emitFunction(func);

        if (module_->globals.empty()) return;

        out_ << "\n.data\n";
        for (auto* gv : module_->globals)
        {
            out_ << gv->name << ":\n";
            if (gv->isScalar())
            {
                if (gv->type == I32 || gv->type == F32)
                {
                    int val = gv->initVals.empty() ? 0 : gv->initVals[0];
                    out_ << "  .word " << val << "\n";
                }
                else if (gv->type == I64 || gv->type == PTR || gv->type == F64)
                {
                    long long val = gv->initVals.empty() ? 0 : (long long)gv->initVals[0];
                    out_ << "  .quad " << val << "\n";
                }
                continue;
            }
            int total_elems = 1;
            for (int d : gv->dims) total_elems *= d;
            bool is32 = (gv->type == I32 || gv->type == F32);
            int  sz   = is32 ? 4 : 8;
            if (gv->initVals.empty())
                out_ << "  .zero " << (total_elems * sz) << "\n";
            else
            {
                int zero_cum = 0;
                for (int v : gv->initVals)
                {
                    if (v == 0)
                    {
                        zero_cum += sz;
                        continue;
                    }
                    if (zero_cum)
                    {
                        out_ << "  .zero " << zero_cum << "\n";
                        zero_cum = 0;
                    }
                    if (is32)
                        out_ << "  .word " << v << "\n";
                    else
                        out_ << "  .quad " << v << "\n";
                }
                if (zero_cum) out_ << "  .zero " << zero_cum << "\n";
            }
        }
    }

    void Codegen::emitFunction(BE::Function* func)
    {
        cur_func_ = func;

        out_ << "\n";
        out_ << ".globl " << func->name << "\n";
        out_ << func->name << ":\n";

        for (auto& [id, block] : func->blocks) emitBlock(block);
    }

    void Codegen::emitBlock(BE::Block* block)
    {
        out_ << "." << cur_func_->name << "_" << block->blockId << ":\n";
        for (auto* inst : block->insts) emitInstruction(inst);
    }

    void Codegen::emitInstruction(BE::MInstruction* minst)
    {
        if (minst->kind == BE::InstKind::MOVE)
        {
            auto* moveInst = static_cast<BE::MoveInst*>(minst);
            out_ << "  MOVE " << formatOperand(moveInst->dest) << ", " << formatOperand(moveInst->src) << "\n";
            if (minst->comment != "") out_ << "  // " << minst->comment << "\n";
            return;
        }

        if (minst->kind != BE::InstKind::TARGET)
            ERROR("Unhandled non-target instruction %u in assembly generation.", static_cast<unsigned>(minst->kind));

        auto*             inst = static_cast<Instr*>(minst);
        std::stringstream ss;

        if (inst->op == Operator::LA)
        {
            auto* dst = static_cast<RegOperand*>(inst->operands[0]);
            auto* sym = static_cast<SymbolOperand*>(inst->operands[1]);
            ss << "  ldr " << formatOperand(dst) << ", =" << sym->name;
            out_ << ss.str() << "\n";
            return;
        }

        auto opAsm  = getOpInfoAsm(inst->op);
        auto opType = getOpInfoType(inst->op);

        ss << "  " << opAsm;

        switch (opType)
        {
            case OpType::L:
            {
                auto* lb = static_cast<LabelOperand*>(inst->operands[0]);
                ss << " " << "." << cur_func_->name << "_" << lb->targetBlockId;
                break;
            }
            case OpType::SYM:
            {
                auto* so = static_cast<SymbolOperand*>(inst->operands[0]);
                ss << " " << so->name;
                break;
            }
            case OpType::P:
            {
                if (inst->op == Operator::STP)
                {
                    auto* r1 = static_cast<RegOperand*>(inst->operands[0]);
                    auto* r2 = static_cast<RegOperand*>(inst->operands[1]);
                    auto* rb = static_cast<RegOperand*>(inst->operands[2]);
                    auto* io = static_cast<ImmeOperand*>(inst->operands[3]);
                    ss << " " << formatOperand(r1) << ", " << formatOperand(r2) << ", [" << formatOperand(rb) << ", #"
                       << io->value << "]!";
                }
                else if (inst->op == Operator::LDP)
                {
                    auto* r1 = static_cast<RegOperand*>(inst->operands[0]);
                    auto* r2 = static_cast<RegOperand*>(inst->operands[1]);
                    auto* rb = static_cast<RegOperand*>(inst->operands[2]);
                    auto* io = static_cast<ImmeOperand*>(inst->operands[3]);
                    ss << " " << formatOperand(r1) << ", " << formatOperand(r2) << ", [" << formatOperand(rb) << "], #"
                       << io->value;
                }
                break;
            }
            case OpType::R2:
                if (inst->op == Operator::LA)
                {
                    auto* dst = static_cast<RegOperand*>(inst->operands[0]);
                    auto* sym = static_cast<SymbolOperand*>(inst->operands[1]);
                    ss << " " << formatOperand(dst) << ", =" << sym->name;
                }
                else if (inst->op == Operator::CMP)
                {
                    if (dynamic_cast<ImmeOperand*>(inst->operands[1]) != nullptr)
                        ss << " " << formatOperand(inst->operands[0]) << ", #"
                           << static_cast<ImmeOperand*>(inst->operands[1])->value;
                    else
                        ss << " " << formatOperand(inst->operands[0]) << ", " << formatOperand(inst->operands[1]);
                }
                else if (inst->op == Operator::CSET)
                {
                    auto*       dst  = static_cast<RegOperand*>(inst->operands[0]);
                    auto*       cc   = static_cast<ImmeOperand*>(inst->operands[1]);
                    const char* cond = "eq";
                    switch (cc->value)
                    {
                        case 0: cond = "eq"; break;   // Equal
                        case 1: cond = "ne"; break;   // Not Equal
                        case 2: cond = "hs"; break;   // Unsigned >= (HS/CS)
                        case 3: cond = "lo"; break;   // Unsigned < (LO/CC)
                        case 8: cond = "hi"; break;   // Unsigned >
                        case 9: cond = "ls"; break;   // Unsigned <=
                        case 10: cond = "ge"; break;  // Signed >=
                        case 11: cond = "lt"; break;  // Signed <
                        case 12: cond = "gt"; break;  // Signed >
                        case 13: cond = "le"; break;  // Signed <=
                        default: cond = "eq"; break;
                    }
                    ss << " " << formatOperand(dst) << ", " << cond;
                }
                else if (inst->op == Operator::MOV)
                {
                    auto* dst = static_cast<RegOperand*>(inst->operands[0]);
                    auto* src = static_cast<RegOperand*>(inst->operands[1]);
                    bool  isFloatMove =
                        (dst->reg.dt == F32 || dst->reg.dt == F64) && (src->reg.dt == F32 || src->reg.dt == F64);
                    if (isFloatMove)
                    {
                        ss.str("");
                        ss << "  fmov " << formatOperand(inst->operands[0]) << ", " << formatOperand(inst->operands[1]);
                    }
                    else { ss << " " << formatOperand(inst->operands[0]) << ", " << formatOperand(inst->operands[1]); }
                }
                else
                    ss << " " << formatOperand(inst->operands[0]) << ", " << formatOperand(inst->operands[1]);
                break;
            case OpType::R:
            {
                if (inst->op == Operator::MOVZ || inst->op == Operator::MOVK || inst->op == Operator::MOVN)
                {
                    ss << " " << formatOperand(inst->operands[0]);
                    if (inst->operands.size() >= 2 && dynamic_cast<ImmeOperand*>(inst->operands[1]) != nullptr)
                    {
                        ss << ", #" << static_cast<ImmeOperand*>(inst->operands[1])->value;
                        if (inst->operands.size() >= 3 && dynamic_cast<ImmeOperand*>(inst->operands[2]) != nullptr)
                        {
                            ss << ", lsl #" << static_cast<ImmeOperand*>(inst->operands[2])->value;
                        }
                    }
                }
                else
                {
                    ss << " " << formatOperand(inst->operands[0]) << ", " << formatOperand(inst->operands[1]) << ", ";
                    if (inst->operands.size() >= 3)
                    {
                        if (dynamic_cast<ImmeOperand*>(inst->operands[2]) != nullptr)
                            ss << "#" << static_cast<ImmeOperand*>(inst->operands[2])->value;
                        else
                            ss << formatOperand(inst->operands[2]);
                    }
                }
                break;
            }
            case OpType::M:
                ss << " " << formatOperand(inst->operands[0]) << ", " << formatOperand(inst->operands[1]);
                break;
            case OpType::Z:
            {
                if (inst->op == Operator::RET)
                    ;
                else if (inst->op == Operator::STP)
                {
                    auto* r1 = static_cast<RegOperand*>(inst->operands[0]);
                    auto* r2 = static_cast<RegOperand*>(inst->operands[1]);
                    auto* rb = static_cast<RegOperand*>(inst->operands[2]);
                    auto* io = static_cast<ImmeOperand*>(inst->operands[3]);
                    ss << " " << formatOperand(r1) << ", " << formatOperand(r2) << ", [" << formatOperand(rb) << ", #"
                       << io->value << "]!";
                }
                else if (inst->op == Operator::LDP)
                {
                    auto* r1 = static_cast<RegOperand*>(inst->operands[0]);
                    auto* r2 = static_cast<RegOperand*>(inst->operands[1]);
                    auto* rb = static_cast<RegOperand*>(inst->operands[2]);
                    auto* io = static_cast<ImmeOperand*>(inst->operands[3]);
                    ss << " " << formatOperand(r1) << ", " << formatOperand(r2) << ", [" << formatOperand(rb) << "], "
                       << "#" << io->value;
                }
                break;
            }
        }
        out_ << ss.str();

        if (minst->comment != "") out_ << "  // " << minst->comment;

        out_ << "\n";
    }
}  // namespace BE::AArch64
