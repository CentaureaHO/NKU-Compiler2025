#ifndef __BACKEND_COMMON_CFG_H__
#define __BACKEND_COMMON_CFG_H__

#include <backend/mir/m_block.h>
#include <map>
#include <vector>

namespace BE::MIR
{
    class CFG
    {
      public:
        std::map<uint32_t, BE::Block*>       blocks;
        std::vector<std::vector<BE::Block*>> graph;
        std::vector<std::vector<BE::Block*>> inv_graph;
        std::vector<std::vector<uint32_t>>   graph_id;
        std::vector<std::vector<uint32_t>>   inv_graph_id;

        uint32_t   max_label;
        BE::Block* entry_block;
        BE::Block* ret_block;

      public:
        CFG();
        ~CFG() = default;

        void addNewBlock(uint32_t id, BE::Block* block);
        void makeEdge(uint32_t from, uint32_t to);
        void removeEdge(uint32_t from, uint32_t to);

        std::vector<std::vector<uint32_t>> buildGraphAdjacencyList() const;
    };
}  // namespace BE::MIR

#endif  // __BACKEND_COMMON_CFG_H__
