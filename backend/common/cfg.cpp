#include <backend/common/cfg.h>
#include <algorithm>

namespace BE::MIR
{
    CFG::CFG() : max_label(0), entry_block(nullptr), ret_block(nullptr) {}

    void CFG::addNewBlock(uint32_t id, BE::Block* block)
    {
        blocks[id] = block;
        if (id > max_label) max_label = id;

        if (graph.size() <= id)
        {
            graph.resize(id + 1);
            inv_graph.resize(id + 1);
            graph_id.resize(id + 1);
            inv_graph_id.resize(id + 1);
        }
    }

    void CFG::makeEdge(uint32_t from, uint32_t to)
    {
        uint32_t max_id = std::max(from, to);
        if (graph.size() <= max_id)
        {
            graph.resize(max_id + 1);
            inv_graph.resize(max_id + 1);
            graph_id.resize(max_id + 1);
            inv_graph_id.resize(max_id + 1);
        }

        if (blocks.find(from) == blocks.end() || blocks.find(to) == blocks.end()) return;

        if (std::find(graph_id[from].begin(), graph_id[from].end(), to) == graph_id[from].end())
        {
            graph[from].push_back(blocks[to]);
            graph_id[from].push_back(to);
            inv_graph[to].push_back(blocks[from]);
            inv_graph_id[to].push_back(from);
        }
    }

    void CFG::removeEdge(uint32_t from, uint32_t to)
    {
        if (from >= graph_id.size() || to >= inv_graph_id.size()) return;

        auto it_id = std::find(graph_id[from].begin(), graph_id[from].end(), to);
        if (it_id != graph_id[from].end())
        {
            size_t idx = std::distance(graph_id[from].begin(), it_id);
            graph_id[from].erase(it_id);
            graph[from].erase(graph[from].begin() + idx);
        }

        auto it_inv_id = std::find(inv_graph_id[to].begin(), inv_graph_id[to].end(), from);
        if (it_inv_id != inv_graph_id[to].end())
        {
            size_t idx = std::distance(inv_graph_id[to].begin(), it_inv_id);
            inv_graph_id[to].erase(it_inv_id);
            inv_graph[to].erase(inv_graph[to].begin() + idx);
        }
    }

    std::vector<std::vector<uint32_t>> CFG::buildGraphAdjacencyList() const { return graph_id; }
}  // namespace BE::MIR
