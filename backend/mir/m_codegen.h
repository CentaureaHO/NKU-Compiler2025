#ifndef __BACKEND_MIR_M_CODEGEN_H__
#define __BACKEND_MIR_M_CODEGEN_H__

#include <backend/mir/m_module.h>
#include <backend/mir/m_function.h>
#include <backend/mir/m_block.h>
#include <backend/mir/m_instruction.h>
#include <backend/mir/m_defs.h>
#include <iostream>

namespace BE
{
    class MCodeGen
    {
      public:
        MCodeGen(Module* module, std::ostream& output)
            : module_(module), out_(output), cur_func_(nullptr), cur_block_(nullptr)
        {}

        virtual ~MCodeGen() = default;

        virtual void generateAssembly() = 0;

      protected:
        Module*       module_;
        std::ostream& out_;
        Function*     cur_func_;
        Block*        cur_block_;

        virtual void printHeader()                        = 0;
        virtual void printFunctions()                     = 0;
        virtual void printFunction(Function* func)        = 0;
        virtual void printBlock(Block* block)             = 0;
        virtual void printInstruction(MInstruction* inst) = 0;
        virtual void printGlobalDefinitions()             = 0;

        virtual void printOperand(const Register& reg) = 0;
        virtual void printOperand(Operand* op)         = 0;

        virtual void printPseudoMove(MoveInst* inst);
    };
}  // namespace BE

#endif  // __BACKEND_MIR_M_CODEGEN_H__
