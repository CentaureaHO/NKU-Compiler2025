#ifndef __BACKEND_TARGET_TARGET_H__
#define __BACKEND_TARGET_TARGET_H__

#include <backend/dag/selection_dag.h>
#include <backend/dag/dag_builder.h>
#include <backend/mir/m_defs.h>
#include <string>
#include <memory>
#include <map>

namespace ME
{
    class Module;
    class Block;
}  // namespace ME
namespace BE
{
    class Module;
}

namespace BE::Targeting
{
    class BackendTarget
    {
      public:
        std::map<const ME::Block*, BE::DAG::SelectionDAG*> block_dags;

        virtual ~BackendTarget()
        {
            for (auto& [_, dag] : block_dags)
            {
                if (!dag) continue;
                delete dag;
                dag = nullptr;
            }
        }

        virtual const char* getName() const = 0;

        void buildDAG(ME::Module* ir)
        {
            DAG::DAGBuilder builder;
            for (auto* f : ir->functions)
            {
                if (!f) continue;

                for (auto& [bid, ir_block] : f->blocks)
                {
                    auto* dag = new DAG::SelectionDAG();
                    builder.build(*ir_block, *dag);
                    block_dags[ir_block] = dag;
                }
            }
        }
        virtual void runPipeline(ME::Module* ir, BE::Module* backend, std::ostream* out) = 0;
    };
}  // namespace BE::Targeting

#endif  // __BACKEND_TARGET_TARGET_H__
