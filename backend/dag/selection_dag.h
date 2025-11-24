#ifndef __BACKEND_DAG_SELECTION_DAG_H__
#define __BACKEND_DAG_SELECTION_DAG_H__

#include <backend/dag/sd_node.h>
#include <backend/dag/folding_set.h>
#include <backend/dag/isd.h>
#include <vector>
#include <memory>
#include <unordered_map>

namespace BE
{
    namespace DAG
    {
        class SelectionDAG
        {
            std::vector<SDNode*>                          nodes_;
            uint32_t                                      next_id_ = 0;
            std::unordered_map<FoldingSetNodeID, SDNode*> folding_set_;

          public:
            SelectionDAG() = default;
            ~SelectionDAG()
            {
                for (auto* n : nodes_)
                {
                    if (!n) continue;
                    delete n;
                    n = nullptr;
                }
            }

            SDValue getNode(uint32_t opcode, const std::vector<DataType*>& vts, const std::vector<SDValue>& ops)
            {
                SDNode temp(opcode, vts, ops);

                FoldingSetNodeID ID;
                temp.Profile(ID);

                auto it = folding_set_.find(ID);
                if (it != folding_set_.end()) return SDValue(it->second, 0);

                auto* n = new SDNode(opcode, vts, ops);
                nodes_.push_back(n);
                n->setId(next_id_++);

                folding_set_[ID] = n;

                return SDValue(n, 0);
            }

            SDValue getSymNode(uint32_t opcode, const std::vector<DataType*>& vts, const std::vector<SDValue>& ops,
                const std::string& symbol)
            {
                SDNode temp(opcode, vts, ops);
                temp.setSymbol(symbol);

                FoldingSetNodeID ID;
                temp.Profile(ID);

                auto it = folding_set_.find(ID);
                if (it != folding_set_.end()) return SDValue(it->second, 0);

                auto* n = new SDNode(opcode, vts, ops);
                n->setSymbol(symbol);
                nodes_.push_back(n);
                n->setId(next_id_++);

                folding_set_[ID] = n;

                return SDValue(n, 0);
            }

            SDValue getImmNode(
                uint32_t opcode, const std::vector<DataType*>& vts, const std::vector<SDValue>& ops, int64_t imm)
            {
                SDNode temp(opcode, vts, ops);
                temp.setImmI64(imm);

                FoldingSetNodeID ID;
                temp.Profile(ID);

                auto it = folding_set_.find(ID);
                if (it != folding_set_.end()) return SDValue(it->second, 0);

                auto* n = new SDNode(opcode, vts, ops);
                n->setImmI64(imm);
                nodes_.push_back(n);
                n->setId(next_id_++);

                folding_set_[ID] = n;

                return SDValue(n, 0);
            }

            SDValue getFrameIndexNode(int frame_index, DataType* ptr_ty)
            {
                SDNode temp(static_cast<unsigned>(ISD::FRAME_INDEX), {ptr_ty}, {});
                temp.setFrameIndex(frame_index);

                FoldingSetNodeID ID;
                temp.Profile(ID);

                auto it = folding_set_.find(ID);
                if (it != folding_set_.end()) return SDValue(it->second, 0);

                auto* n = new SDNode(static_cast<unsigned>(ISD::FRAME_INDEX), {ptr_ty}, {});
                n->setFrameIndex(frame_index);
                nodes_.push_back(n);
                n->setId(next_id_++);

                folding_set_[ID] = n;

                return SDValue(n, 0);
            }

            SDValue getRegNode(size_t ir_reg_id, DataType* vt)
            {
                SDNode temp(static_cast<unsigned>(ISD::REG), {vt}, {});
                temp.setIRRegId(ir_reg_id);

                FoldingSetNodeID ID;
                temp.Profile(ID);

                auto it = folding_set_.find(ID);
                if (it != folding_set_.end()) return SDValue(it->second, 0);

                auto* n = new SDNode(static_cast<unsigned>(ISD::REG), {vt}, {});
                n->setIRRegId(ir_reg_id);
                nodes_.push_back(n);
                n->setId(next_id_++);

                folding_set_[ID] = n;

                return SDValue(n, 0);
            }

            SDValue getConstantI64(int64_t value, DataType* vt)
            {
                SDNode temp(static_cast<unsigned>(ISD::CONST_I64), {vt}, {});
                temp.setImmI64(value);

                FoldingSetNodeID ID;
                temp.Profile(ID);

                auto it = folding_set_.find(ID);
                if (it != folding_set_.end()) { return SDValue(it->second, 0); }

                auto* n = new SDNode(static_cast<unsigned>(ISD::CONST_I64), {vt}, {});
                n->setImmI64(value);
                nodes_.push_back(n);
                n->setId(next_id_++);
                folding_set_[ID] = n;

                return SDValue(n, 0);
            }

            SDValue getConstantF32(float value, DataType* vt)
            {
                SDNode temp(static_cast<unsigned>(ISD::CONST_F32), {vt}, {});
                temp.setImmF32(value);

                FoldingSetNodeID ID;
                temp.Profile(ID);

                auto it = folding_set_.find(ID);
                if (it != folding_set_.end()) { return SDValue(it->second, 0); }

                auto* n = new SDNode(static_cast<unsigned>(ISD::CONST_F32), {vt}, {});
                n->setImmF32(value);
                nodes_.push_back(n);
                n->setId(next_id_++);
                folding_set_[ID] = n;

                return SDValue(n, 0);
            }

            const std::vector<SDNode*>& getNodes() const { return nodes_; }
        };

    }  // namespace DAG
}  // namespace BE

#endif  // __BACKEND_DAG_SELECTION_DAG_H__
