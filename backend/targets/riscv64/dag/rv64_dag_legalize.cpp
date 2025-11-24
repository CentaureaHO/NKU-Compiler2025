#include <backend/targets/riscv64/dag/rv64_dag_legalize.h>
#include <backend/dag/isd.h>
#include <interfaces/middleend/ir_defs.h>

namespace BE
{
    namespace RV64
    {

        static ME::ICmpOp swapPredicate(ME::ICmpOp cc)
        {
            switch (cc)
            {
                case ME::ICmpOp::EQ: return ME::ICmpOp::EQ;
                case ME::ICmpOp::NE: return ME::ICmpOp::NE;
                case ME::ICmpOp::SLT: return ME::ICmpOp::SGT;
                case ME::ICmpOp::SGT: return ME::ICmpOp::SLT;
                case ME::ICmpOp::SLE: return ME::ICmpOp::SGE;
                case ME::ICmpOp::SGE: return ME::ICmpOp::SLE;
                case ME::ICmpOp::ULT: return ME::ICmpOp::UGT;
                case ME::ICmpOp::UGT: return ME::ICmpOp::ULT;
                case ME::ICmpOp::ULE: return ME::ICmpOp::UGE;
                case ME::ICmpOp::UGE: return ME::ICmpOp::ULE;
                default: return cc;
            }
        }

        void DAGLegalizer::run(BE::DAG::SelectionDAG& dag) const
        {
            for (const auto& up : dag.getNodes())
            {
                auto* n = const_cast<BE::DAG::SDNode*>(up);
                if (n->getOpcode() != static_cast<unsigned>(BE::DAG::ISD::ICMP)) continue;
                if (n->getNumOperands() != 2) continue;

                auto* lhs = n->getOperand(0).getNode();
                auto* rhs = n->getOperand(1).getNode();
                if (!lhs || !rhs) continue;

                bool lhsIsConst = lhs->getOpcode() == static_cast<unsigned>(BE::DAG::ISD::CONST_I32) ||
                                  lhs->getOpcode() == static_cast<unsigned>(BE::DAG::ISD::CONST_I64) ||
                                  lhs->getOpcode() == static_cast<unsigned>(BE::DAG::ISD::CONST_F32);
                bool rhsIsConst = rhs->getOpcode() == static_cast<unsigned>(BE::DAG::ISD::CONST_I32) ||
                                  rhs->getOpcode() == static_cast<unsigned>(BE::DAG::ISD::CONST_I64) ||
                                  rhs->getOpcode() == static_cast<unsigned>(BE::DAG::ISD::CONST_F32);

                if (lhsIsConst && !rhsIsConst)
                {
                    auto cc      = static_cast<ME::ICmpOp>(n->hasImmI64() ? n->getImmI64() : 0);
                    auto swapped = swapPredicate(cc);
                    n->setImmI64(static_cast<int64_t>(swapped));
                    std::vector<BE::DAG::SDValue> newOps;
                    newOps.push_back(BE::DAG::SDValue(rhs, 0));
                    newOps.push_back(BE::DAG::SDValue(lhs, 0));
                    n->replaceOperands(newOps);
                }
            }
        }

    }  // namespace RV64
}  // namespace BE
