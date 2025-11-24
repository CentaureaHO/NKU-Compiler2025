#ifndef __BACKEND_RA_REGISTER_ALLOCATOR_H__
#define __BACKEND_RA_REGISTER_ALLOCATOR_H__

#include <backend/mir/m_module.h>
#include <backend/target/target_reg_info.h>
#include <backend/target/target_instr_adapter.h>

namespace BE::RA
{
    template <class Impl>
    class RegisterAllocator
    {
      public:
        void allocate(BE::Module& module, const BE::Targeting::TargetRegInfo& regInfo)
        {
            for (auto* func : module.functions) static_cast<Impl*>(this)->allocateFunction(*func, regInfo);
        }
    };
}  // namespace BE::RA

#endif  // __BACKEND_RA_REGISTER_ALLOCATOR_H__
