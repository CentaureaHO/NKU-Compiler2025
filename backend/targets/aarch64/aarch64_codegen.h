#ifndef __BACKEND_TARGETS_AARCH64_AARCH64_CODEGEN_H__
#define __BACKEND_TARGETS_AARCH64_AARCH64_CODEGEN_H__

#include <backend/mir/m_module.h>
#include <backend/mir/m_codegen.h>
#include <iostream>

namespace BE::AArch64
{
    class Codegen
    {
      public:
        Codegen(BE::Module* module, std::ostream& out) : module_(module), out_(out) {}

        void generateAssembly();

      private:
        BE::Module*   module_;
        std::ostream& out_;

        BE::Function* cur_func_       = nullptr;
        int           cur_stack_size_ = 0;

        void emitFunction(BE::Function* func);
        void emitBlock(BE::Block* block);
        void emitInstruction(BE::MInstruction* inst);
    };
}  // namespace BE::AArch64

#endif  // __BACKEND_TARGETS_AARCH64_AARCH64_CODEGEN_H__
