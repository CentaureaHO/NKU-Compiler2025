#ifndef __BACKEND_ISEL_ISEL_BASE_H__
#define __BACKEND_ISEL_ISEL_BASE_H__

#include <middleend/module/ir_module.h>
#include <middleend/ir_visitor.h>
#include <backend/mir/m_module.h>
#include <map>

namespace BE
{
    template <typename Derived>
    class ISelBase
    {
      public:
        ISelBase(BE::Module* backend_module) : m_backend_module(backend_module) {}
        virtual ~ISelBase() = default;

        void run() { static_cast<Derived*>(this)->runImpl(); }

      protected:
        BE::Module* m_backend_module;

        Derived&       derived() { return static_cast<Derived&>(*this); }
        const Derived& derived() const { return static_cast<const Derived&>(*this); }
    };

    using IRIselBase = ME::Visitor_t<void>;
}  // namespace BE

#endif  // __BACKEND_ISEL_ISEL_BASE_H__
