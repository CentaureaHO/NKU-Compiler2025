#include <backend/targets/riscv64/rv64_codegen.h>
#include <backend/targets/riscv64/rv64_defs.h>
#include <debug.h>
#include <cassert>

namespace BE::RV64
{
    CodeGen::CodeGen(BE::Module* module, std::ostream& output) : BE::MCodeGen(module, output) {}

    void CodeGen::generateAssembly()
    {
        printHeader();
        printFunctions();
        printGlobalDefinitions();
    }

    void CodeGen::printHeader()
    {
        out_ << "\t.text\n\t.globl main\n";
        out_ << "\t.attribute	4, 16\n";
        out_ << "\t.attribute arch, \"rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0\"\n\n";
    }

    void CodeGen::printFunctions()
    {
        for (auto& func : module_->functions) { printFunction(func); }
    }

    void CodeGen::printFunction(BE::Function* func)
    {
        cur_func_ = func;
        out_ << func->name << ":\n";

        for (auto& [blockId, block] : func->blocks) { printBlock(block); }
    }

    void CodeGen::printBlock(BE::Block* block)
    {
        cur_block_   = block;
        uint32_t bid = block->blockId;

        out_ << "." << cur_func_->name << "_" << bid << ":\n";

        for (auto& inst : block->insts)
        {
            out_ << "\t";
            printInstruction(inst);
            out_ << "\n";
        }
    }

    void CodeGen::printInstruction(BE::MInstruction* inst)
    {
        if (auto* ti = dynamic_cast<Instr*>(inst))
        {
            printASM(ti);
            return;
        }
        if (auto* mv = dynamic_cast<MoveInst*>(inst))
        {
            printPseudoMove(mv);
            return;
        }
        if (auto* phi = dynamic_cast<PhiInst*>(inst))
        {
            printOperand(phi->resReg);
            out_ << " = phi ";
            for (auto& [labelId, srcOp] : phi->incomingVals)
            {
                out_ << "[" << labelId << " -> ";
                printOperand(srcOp);
                out_ << "], ";
            }
            return;
        }
        ERROR("Unsupported instruction kind in code generation");
    }

    void CodeGen::printASM(Instr* inst)
    {
        auto isMemInst = [](Operator op) {
            return op == Operator::LW || op == Operator::LD || op == Operator::FLW || op == Operator::FLD ||
                   op == Operator::SW || op == Operator::SD || op == Operator::FSW || op == Operator::FSD;
        };

        std::string opAsm  = getOpInfoAsm(inst->op);
        OpType      opType = getOpInfoType(inst->op);

        out_ << opAsm;
        if (opAsm.length() <= 3)
            out_ << "\t\t";
        else
            out_ << "\t";

        switch (opType)
        {
            case OpType::R:
            {
                printOperand(inst->rd);
                out_ << ", ";
                printOperand(inst->rs1);
                out_ << ", ";
                printOperand(inst->rs2);
                break;
            }
            case OpType::R2:
            {
                printOperand(inst->rd);
                out_ << ", ";
                printOperand(inst->rs1);
                if (inst->op == Operator::FCVT_W_S) out_ << ", rtz";
                break;
            }
            case OpType::I:
            {
                printOperand(inst->rd);
                out_ << ", ";

                if (isMemInst(inst->op))
                {
                    if (inst->use_ops && inst->fiop)
                        printOperand(inst->fiop);  // Use fiop (could be FrameIndexOperand)
                    else
                        out_ << inst->imme;
                    out_ << "(";
                    printOperand(inst->rs1);
                    out_ << ")";
                }
                else
                {
                    printOperand(inst->rs1);
                    out_ << ", ";
                    if (inst->use_ops && inst->fiop)
                        printOperand(inst->fiop);  // Use fiop (could be FrameIndexOperand)
                    else
                        out_ << inst->imme;
                }
                break;
            }
            case OpType::S:
            {
                printOperand(inst->rs1);
                out_ << ", ";
                if (inst->use_ops && inst->fiop)
                    printOperand(inst->fiop);  // Use fiop (could be FrameIndexOperand)
                else
                    out_ << inst->imme;
                out_ << "(";
                printOperand(inst->rs2);
                out_ << ")";
                break;
            }
            case OpType::B:
            {
                printOperand(inst->rs1);
                out_ << ", ";
                printOperand(inst->rs2);
                out_ << ", ";
                if (inst->use_label)
                    printOperand(inst->label);
                else
                    out_ << "." << cur_func_->name << "_" << inst->imme;
                break;
            }
            case OpType::U:
            {
                printOperand(inst->rd);
                out_ << ", ";
                if (inst->use_label)
                    printOperand(inst->label);
                else
                    out_ << inst->imme;
                break;
            }
            case OpType::J:
            {
                printOperand(inst->rd);
                out_ << ", ";
                if (inst->use_label)
                    printOperand(inst->label);
                else
                    out_ << "." << cur_func_->name << "_" << inst->imme;
                break;
            }
            case OpType::CALL:
            {
                out_ << inst->func_name;
                break;
            }
            default: ERROR("Unsupported RV64 instruction type");
        }

        if (!inst->comment.empty()) out_ << "\t# " << inst->comment;
    }

    void CodeGen::printOperand(const Register& reg)
    {
        if (reg.isVreg)
            out_ << "v_" << reg.rId << "_" << reg.dt->toString();
        else
        {
            static const char* regAliases[64] = {"x0",
                "ra",
                "sp",
                "gp",
                "tp",
                "t0",
                "t1",
                "t2",
                "fp",
                "s1",
                "a0",
                "a1",
                "a2",
                "a3",
                "a4",
                "a5",
                "a6",
                "a7",
                "s2",
                "s3",
                "s4",
                "s5",
                "s6",
                "s7",
                "s8",
                "s9",
                "s10",
                "s11",
                "t3",
                "t4",
                "t5",
                "t6",
                "ft0",
                "ft1",
                "ft2",
                "ft3",
                "ft4",
                "ft5",
                "ft6",
                "ft7",
                "fs0",
                "fs1",
                "fa0",
                "fa1",
                "fa2",
                "fa3",
                "fa4",
                "fa5",
                "fa6",
                "fa7",
                "fs2",
                "fs3",
                "fs4",
                "fs5",
                "fs6",
                "fs7",
                "fs8",
                "fs9",
                "fs10",
                "fs11",
                "ft8",
                "ft9",
                "ft10",
                "ft11"};

            ASSERT(reg.rId < 64 && "Register ID out of range");
            out_ << regAliases[reg.rId];
        }
    }

    void CodeGen::printOperand(const Label& label)
    {
        if (label.is_data)
        {
            if (label.is_la)
                out_ << label.name;
            else if (label.is_hi)
                out_ << "%hi(" << label.name << ")";
            else
                out_ << "%lo(" << label.name << ")";
        }
        else
            out_ << "." << cur_func_->name << "_" << label.jmp_label;
    }

    void CodeGen::printOperand(BE::Operand* op)
    {
        switch (op->ot)
        {
            case BE::Operand::Type::REG:
            {
                RegOperand* regOp = static_cast<RegOperand*>(op);
                printOperand(regOp->reg);
                break;
            }
            case BE::Operand::Type::IMMI32:
            {
                I32Operand* immOp = static_cast<I32Operand*>(op);
                out_ << immOp->val;
                break;
            }
            case BE::Operand::Type::IMMF32:
            {
                F32Operand* immOp = static_cast<F32Operand*>(op);
                out_ << immOp->val;
                break;
            }
            case BE::Operand::Type::FRAME_INDEX:
            {
                FrameIndexOperand* fiOp = static_cast<FrameIndexOperand*>(op);
                out_ << "[FI#" << fiOp->frameIndex << "]";
                break;
            }
            default: ERROR("Unsupported operand type in printing");
        }
    }

    void CodeGen::printGlobalDefinitions()
    {
        out_ << "\t.data\n";

        for (auto* gv : module_->globals)
        {
            out_ << gv->name << ":\n";

            if (gv->isScalar())
            {
                if (gv->type == I32)
                {
                    if (gv->initVals.empty())
                        out_ << "\t.word\t0\n";
                    else
                        out_ << "\t.word\t" << gv->initVals[0] << "\n";
                }
                else if (gv->type == F32)
                {
                    int bits = gv->initVals.empty() ? 0 : gv->initVals[0];
                    out_ << "\t.word\t" << bits << "\n";
                }
                else if (gv->type == I64 || gv->type == PTR)
                {
                    long long val = gv->initVals.empty() ? 0 : (long long)gv->initVals[0];
                    out_ << "\t.dword\t" << val << "\n";
                }
                else if (gv->type == F64) { out_ << "\t.dword\t0\n"; }
                continue;
            }

            int total_elems = 1;
            for (int d : gv->dims) total_elems *= d;

            bool is_i32  = (gv->type == I32);
            bool is_f32  = (gv->type == F32);
            int  elem_sz = is_i32 || is_f32 ? 4 : 8;

            if (gv->initVals.empty())
                out_ << "\t.zero\t" << (total_elems * elem_sz) << "\n";
            else
            {
                int zero_cum = 0;
                for (int v : gv->initVals)
                {
                    if (v == 0)
                    {
                        zero_cum += elem_sz;
                        continue;
                    }
                    if (zero_cum)
                    {
                        out_ << "\t.zero\t" << zero_cum << "\n";
                        zero_cum = 0;
                    }
                    if (is_i32 || is_f32)
                        out_ << "\t.word\t" << v << "\n";
                    else
                        out_ << "\t.dword\t" << v << "\n";
                }
                if (zero_cum) out_ << "\t.zero\t" << zero_cum << "\n";
            }
        }
    }

    std::string CodeGen::getOpInfoAsm(Operator op)
    {
        switch (op)
        {
#define X(name, type, _asm, latency) \
    case Operator::name: return #_asm;
            RV64_INSTS
#undef X
            default: ERROR("Unknown operator: %d", (int)op);
        }
    }

    OpType CodeGen::getOpInfoType(Operator op)
    {
        switch (op)
        {
#define X(name, type, _asm, latency) \
    case Operator::name: return OpType::type;
            RV64_INSTS
#undef X
            default: ERROR("Unknown operator: %d", (int)op);
        }
    }
}  // namespace BE::RV64
