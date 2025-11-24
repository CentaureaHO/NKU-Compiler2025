#ifndef __BACKEND_TARGETS_AARCH64_DAG_AARCH64_DAG_LEGALIZE_H__
#define __BACKEND_TARGETS_AARCH64_DAG_AARCH64_DAG_LEGALIZE_H__

#include <backend/dag/selection_dag.h>

namespace BE::AArch64
{
    class DAGLegalizer
    {
      public:
        void run(BE::DAG::SelectionDAG& dag) const;
    };
}  // namespace BE::AArch64

#endif  // __BACKEND_TARGETS_AARCH64_DAG_AARCH64_DAG_LEGALIZE_H__
