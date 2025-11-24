#ifndef __BACKEND_RA_LINEAR_SCAN_H__
#define __BACKEND_RA_LINEAR_SCAN_H__

#include <backend/ra/register_allocator.h>
#include <map>

namespace BE::RA
{
    class LinearScanRA : public RegisterAllocator<LinearScanRA>
    {
      public:
        void allocateFunction(BE::Function& func, const BE::Targeting::TargetRegInfo& regInfo);
    };
}  // namespace BE::RA

#endif  // __BACKEND_RA_LINEAR_SCAN_H__
