#ifndef __BACKEND_RV64_RV64_CODEGEN_H__
#define __BACKEND_RV64_RV64_CODEGEN_H__

#include <backend/mir/m_codegen.h>
#include <backend/mir/m_module.h>
#include <backend/targets/riscv64/rv64_defs.h>

namespace BE::RV64
{
    class CodeGen : public BE::MCodeGen
    {
      public:
        CodeGen(BE::Module* module, std::ostream& output);

        void generateAssembly() override;

      protected:
        void printHeader() override;
        void printFunctions() override;
        void printFunction(BE::Function* func) override;
        void printBlock(BE::Block* block) override;
        void printInstruction(BE::MInstruction* inst) override;
        void printGlobalDefinitions() override;

        void printOperand(const Register& reg) override;
        void printOperand(BE::Operand* op) override;

      private:
        void printASM(Instr* inst);
        void printOperand(const Label& label);

        std::string getOpInfoAsm(Operator op);
        OpType      getOpInfoType(Operator op);
    };
}  // namespace BE::RV64

#endif  // __BACKEND_RV64_RV64_CODEGEN_H__
