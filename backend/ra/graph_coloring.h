#ifndef __BACKEND_RA_GRAPH_COLORING_H__
#define __BACKEND_RA_GRAPH_COLORING_H__

#include <backend/ra/register_allocator.h>
#include <debug.h>

namespace BE::RA
{
    class GraphColoringRA : public RegisterAllocator<GraphColoringRA>
    {
      public:
        void allocateFunction(BE::Function& func, const BE::Targeting::TargetRegInfo& regInfo)
        {
            (void)func;
            (void)regInfo;
            TODO("Implement GraphColoringRA");
        }
    };
}  // namespace BE::RA

#endif  // __BACKEND_RA_GRAPH_COLORING_H__
