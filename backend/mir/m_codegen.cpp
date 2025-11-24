#include <backend/mir/m_codegen.h>

namespace BE
{
    void MCodeGen::printPseudoMove(MoveInst* inst)
    {
        out_ << "MOVE ";
        printOperand(inst->dest);
        out_ << ", ";
        printOperand(inst->src);

        if (inst->comment.empty()) return;
        out_ << "\t# " << inst->comment;
    }

}  // namespace BE
