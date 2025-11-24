#ifndef __BACKEND_DAG_TARGET_LOWERING_H__
#define __BACKEND_DAG_TARGET_LOWERING_H__

#include <backend/dag/selection_dag.h>

namespace BE
{
    namespace DAG
    {
        class TargetLowering
        {
          public:
            virtual ~TargetLowering() = default;

            virtual void lower(SelectionDAG& dag) const = 0;
        };

    }  // namespace DAG
}  // namespace BE

#endif  // __BACKEND_DAG_TARGET_LOWERING_H__
