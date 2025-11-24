#ifndef __BACKEND_TARGETS_RISCV64_DAG_RV64_DAG_LEGALIZE_H__
#define __BACKEND_TARGETS_RISCV64_DAG_RV64_DAG_LEGALIZE_H__

#include <backend/dag/selection_dag.h>

namespace BE
{
    namespace RV64
    {
        class DAGLegalizer
        {
          public:
            void run(BE::DAG::SelectionDAG& dag) const;
        };

    }  // namespace RV64
}  // namespace BE

#endif  // __BACKEND_TARGETS_RISCV64_DAG_RV64_DAG_LEGALIZE_H__
