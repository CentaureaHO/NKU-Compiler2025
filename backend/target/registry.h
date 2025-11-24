#ifndef __BACKEND_TARGET_REGISTRY_H__
#define __BACKEND_TARGET_REGISTRY_H__

#include <string>
#include <vector>
#include <functional>

namespace BE::Targeting
{
    class BackendTarget;

    class TargetRegistry
    {
      public:
        static BackendTarget*           getTarget(const std::string& triple);
        static std::vector<std::string> listTargets();

        static void registerTargetFactory(const std::string& name, std::function<BackendTarget*()> factory);
    };
}  // namespace BE::Targeting

#endif  // __BACKEND_TARGET_REGISTRY_H__
