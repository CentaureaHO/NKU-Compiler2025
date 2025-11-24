#include <backend/target/registry.h>
#include <backend/target/target.h>
#include <map>
#include <string>
#include <vector>
#include <functional>

namespace BE::Targeting
{
    static std::map<std::string, std::function<BackendTarget*()>>& factories()
    {
        static std::map<std::string, std::function<BackendTarget*()>> f;
        return f;
    }

    struct Instances
    {
        std::map<std::string, BackendTarget*> map;
        ~Instances()
        {
            for (auto& [_, target] : map)
            {
                if (!target) continue;
                delete target;
                target = nullptr;
            }
        }
    };

    static std::map<std::string, BackendTarget*>& instances()
    {
        static Instances i;
        return i.map;
    }

    BackendTarget* TargetRegistry::getTarget(const std::string& triple)
    {
        auto itI = instances().find(triple);
        if (itI != instances().end()) return itI->second;

        auto itF = factories().find(triple);
        if (itF != factories().end())
        {
            auto* ptr           = itF->second();
            instances()[triple] = ptr;
            return ptr;
        }
        return nullptr;
    }

    std::vector<std::string> TargetRegistry::listTargets()
    {
        std::vector<std::string> keys;
        keys.reserve(factories().size());
        for (auto& [k, _] : factories()) keys.push_back(k);
        return keys;
    }

    void TargetRegistry::registerTargetFactory(const std::string& name, std::function<BackendTarget*()> factory)
    {
        factories()[name] = std::move(factory);
    }
}  // namespace BE::Targeting
