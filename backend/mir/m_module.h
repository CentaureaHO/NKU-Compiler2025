#ifndef __BACKEND_MIR_M_MODULE_H__
#define __BACKEND_MIR_M_MODULE_H__

#include <backend/mir/m_function.h>
#include <string>
#include <vector>

namespace BE
{
    class GlobalVariable
    {
      public:
        std::string      name;
        DataType*        type;
        std::vector<int> dims;

        std::vector<int> initVals;

        GlobalVariable(DataType* t, const std::string& n) : name(n), type(t), dims(), initVals() {}

        bool isScalar() const { return dims.empty(); }
    };
    class Module
    {
      public:
        std::vector<Function*>       functions;
        std::vector<GlobalVariable*> globals;
    };
}  // namespace BE

#endif  // __BACKEND_MIR_M_MODULE_H__