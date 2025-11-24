#ifndef __BACKEND_DAG_SD_NODE_H__
#define __BACKEND_DAG_SD_NODE_H__

#include <backend/mir/m_defs.h>
#include <backend/dag/folding_set.h>
#include <backend/dag/isd.h>
#include <vector>
#include <cstdint>
#include <string>

namespace BE
{
    namespace DAG
    {
        class SDNode;

        class SDValue
        {
            SDNode*  node_;
            uint32_t res_no_;

          public:
            SDValue() : node_(nullptr), res_no_(0) {}
            SDValue(SDNode* node, uint32_t res_no) : node_(node), res_no_(res_no) {}

            SDNode*  getNode() const { return node_; }
            uint32_t getResNo() const { return res_no_; }
            explicit operator bool() const { return node_ != nullptr; }
        };

        class SDNode
        {
            uint32_t               id_ = 0;
            uint32_t               opcode_;
            std::vector<SDValue>   operands_;
            std::vector<DataType*> value_types_;

            bool        has_imm_i64_ = false;
            int64_t     imm_i64_     = 0;
            bool        has_imm_f32_ = false;
            float       imm_f32_     = 0.0f;
            bool        has_symbol_  = false;
            std::string symbol_;

            bool   has_ir_reg_id_ = false;
            size_t ir_reg_id_     = 0;

            bool has_frame_index_ = false;
            int  frame_index_     = -1;

          public:
            SDNode(uint32_t opcode, const std::vector<DataType*>& vts, const std::vector<SDValue>& ops)
                : opcode_(opcode), operands_(ops), value_types_(vts)
            {}

            void     setId(uint32_t id) { id_ = id; }
            uint32_t getId() const { return id_; }

            uint32_t getOpcode() const { return opcode_; }
            void     setOpcode(uint32_t opc) { opcode_ = opc; }

            const std::vector<SDValue>& getOperands() const { return operands_; }
            const SDValue&              getOperand(unsigned i) const { return operands_[i]; }
            void                        setOperand(unsigned i, const SDValue& v)
            {
                if (i < operands_.size()) operands_[i] = v;
            }
            void replaceOperands(const std::vector<SDValue>& ops) { operands_ = ops; }

            unsigned getNumOperands() const { return operands_.size(); }
            unsigned getNumValues() const { return value_types_.size(); }

            DataType* getValueType(unsigned i) const { return value_types_[i]; }

            void setImmI64(int64_t v)
            {
                has_imm_i64_ = true;
                imm_i64_     = v;
            }
            bool    hasImmI64() const { return has_imm_i64_; }
            int64_t getImmI64() const { return imm_i64_; }

            void setImmF32(float v)
            {
                has_imm_f32_ = true;
                imm_f32_     = v;
            }
            bool  hasImmF32() const { return has_imm_f32_; }
            float getImmF32() const { return imm_f32_; }

            void setSymbol(const std::string& s)
            {
                has_symbol_ = true;
                symbol_     = s;
            }
            bool               hasSymbol() const { return has_symbol_; }
            const std::string& getSymbol() const { return symbol_; }

            void setIRRegId(size_t id)
            {
                has_ir_reg_id_ = true;
                ir_reg_id_     = id;
            }
            bool   hasIRRegId() const { return has_ir_reg_id_; }
            size_t getIRRegId() const { return ir_reg_id_; }

            void setFrameIndex(int fi)
            {
                has_frame_index_ = true;
                frame_index_     = fi;
            }
            bool hasFrameIndex() const { return has_frame_index_; }
            int  getFrameIndex() const { return frame_index_; }

            void Profile(FoldingSetNodeID& ID) const
            {
                ID.AddInteger(opcode_);

                ID.AddInteger(operands_.size());
                ID.AddInteger(value_types_.size());

                for (const auto& op : operands_)
                {
                    ID.AddPointer(op.getNode());
                    ID.AddInteger(op.getResNo());
                }

                for (auto* vt : value_types_) ID.AddPointer(vt);

                if (has_imm_i64_)
                {
                    ID.AddBoolean(true);
                    ID.AddInteger(imm_i64_);
                }
                else
                    ID.AddBoolean(false);

                if (has_imm_f32_)
                {
                    ID.AddBoolean(true);
                    ID.AddFloat(imm_f32_);
                }
                else
                    ID.AddBoolean(false);

                if (has_symbol_)
                {
                    ID.AddBoolean(true);
                    ID.AddString(symbol_);
                }
                else
                    ID.AddBoolean(false);

                if (has_frame_index_)
                {
                    ID.AddBoolean(true);
                    ID.AddInteger(frame_index_);
                }
                else
                    ID.AddBoolean(false);

                if (opcode_ == static_cast<unsigned>(ISD::REG))
                {
                    if (has_ir_reg_id_)
                    {
                        ID.AddBoolean(true);
                        ID.AddInteger(ir_reg_id_);
                    }
                    else
                        ID.AddBoolean(false);
                }
            }
        };

    }  // namespace DAG
}  // namespace BE

#endif  // __BACKEND_DAG_SD_NODE_H__