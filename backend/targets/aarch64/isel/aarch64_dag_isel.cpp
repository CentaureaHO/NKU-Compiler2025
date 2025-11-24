#include <backend/targets/aarch64/isel/aarch64_dag_isel.h>
#include <backend/target/target.h>
#include <backend/dag/selection_dag.h>
#include <backend/dag/isd.h>
#include <backend/dag/dag_builder.h>
#include <backend/targets/aarch64/dag/aarch64_dag_legalize.h>
#include <backend/targets/aarch64/aarch64_defs.h>
#include <middleend/module/ir_instruction.h>
#include <middleend/module/ir_function.h>
#include <debug.h>
#include <transfer.h>
#include <functional>
#include <cstring>

// 在 README 中已经说明了这里可能会出现 I32 op I64 的情况
// 并且 selectBinary 中我也将处理这个情况的代码删去了，请自行考虑如何解决这个问题
// 额外的，在 ../aarch64_def.h 中我定义了 LOC_STR 宏，它可以用于在创建指令时添加位置信息
// 启用这个宏也许可以在你 debug 时帮助你迅速定位到一条错误的指令是在哪里创建的

namespace BE::AArch64
{
    std::vector<const DAG::SDNode*> DAGIsel::scheduleDAG(const DAG::SelectionDAG& dag)
    {
        // ============================================================================
        // TODO: 实现 DAG 调度
        // ============================================================================
        //
        // 作用：
        // 将 DAG 的所有节点排序为线性指令序列，保证依赖关系正确。
        //
        // 为什么需要：
        // - DAG 是无序的节点集合，生成代码前必须确定指令的先后顺序
        // - 必须保证每条指令的操作数在使用前已被计算（拓扑序）
        // - 需要正确处理 Chain 依赖，维持内存操作与副作用的顺序
        //
        // 如何做：
        // - 后序遍历：从根节点（无使用者的节点）出发，先访问依赖再访问当前节点

        TODO("实现 DAG 调度：输出拓扑序的节点列表");
    }

    void DAGIsel::allocateRegistersForNode(const DAG::SDNode* node)
    {
        // ============================================================================
        // 为调度后的 DAG 节点预分配虚拟寄存器
        // ============================================================================
        //
        // 为什么需要：
        // - 在指令选择前，先为每个计算结果分配虚拟寄存器
        // - 确保跨基本块的值（如 PHI 操作数）使用一致的寄存器映射
        // - 分离"分配"与"使用"两个阶段，简化后续的指令生成逻辑
        //
        // 策略：
        // - 若节点对应 IR 寄存器，复用 IR 寄存器映射
        // - 否则分配临时虚拟寄存器

        if (node->getNumValues() == 0) return;

        auto opcode = static_cast<DAG::ISD>(node->getOpcode());

        if (opcode == DAG::ISD::LABEL || opcode == DAG::ISD::SYMBOL || opcode == DAG::ISD::CONST_I32 ||
            opcode == DAG::ISD::CONST_I64 || opcode == DAG::ISD::CONST_F32 || opcode == DAG::ISD::FRAME_INDEX)
            return;

        BE::DataType* dt = node->getValueType(0);
        Register      vreg;

        if (node->hasIRRegId())
            vreg = getOrCreateVReg(node->getIRRegId(), dt);
        else
            vreg = getVReg(dt);

        nodeToVReg_[node] = vreg;
    }

    Register DAGIsel::getOperandReg(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // 操作数获取：统一的实例化入口
        // ============================================================================
        //
        // 作用：为 DAG 节点返回或实例化对应的寄存器
        //
        // 为什么需要：
        // - 指令选择时，需要将 DAG 节点（抽象）映射到寄存器（具体）
        // - 统一处理已分配寄存器、IR 寄存器映射、按需实例化三种情况

        if (!node) ERROR("Cannot get register for null node");

        auto opcode = static_cast<DAG::ISD>(node->getOpcode());

        auto it = nodeToVReg_.find(node);
        if (it != nodeToVReg_.end()) return it->second;

        if (opcode == DAG::ISD::REG && node->hasIRRegId())
            return getOrCreateVReg(node->getIRRegId(), node->getNumValues() > 0 ? node->getValueType(0) : BE::I64);

        if (opcode == DAG::ISD::CONST_I32 || opcode == DAG::ISD::CONST_I64)
        {
            DataType* dt = (node->getNumValues() > 0) ? node->getValueType(0) : BE::I32;
            if (!dt) dt = (opcode == DAG::ISD::CONST_I32) ? BE::I32 : BE::I64;

            Register destReg = getVReg(dt);
            int64_t  imm     = node->hasImmI64() ? node->getImmI64() : 0;

            if (imm == 0)
            {
                Register zeroReg  = (dt == BE::I32) ? PR::wzr : PR::xzr;
                nodeToVReg_[node] = zeroReg;
                return zeroReg;
            }

            auto   segments    = decomposeImm64(static_cast<unsigned long long>(imm));
            size_t numSegments = (dt == BE::I32) ? 2 : 4;

            bool first = true;
            for (size_t i = 0; i < numSegments; ++i)
            {
                if (segments[i] == 0 && !first) continue;
                if (first)
                {
                    m_block->insts.push_back(
                        createInstr2(Operator::MOVZ, new RegOperand(destReg), new ImmeOperand(segments[i])));
                    first = false;
                }
                else
                {
                    m_block->insts.push_back(createInstr3(Operator::MOVK,
                        new RegOperand(destReg),
                        new ImmeOperand(segments[i]),
                        new ImmeOperand(i * 16)));
                }
            }

            nodeToVReg_[node] = destReg;
            return destReg;
        }

        if (opcode == DAG::ISD::CONST_F32)
        {
            DataType* dt = (node->getNumValues() > 0) ? node->getValueType(0) : BE::F32;
            if (!dt) dt = BE::F32;
            Register dstReg = getVReg(dt);

            float fval = node->hasImmF32() ? node->getImmF32() : 0.0f;

            if (fval == 0.0f)
            {
                m_block->insts.push_back(createInstr2(Operator::FMOV, new RegOperand(dstReg), new RegOperand(PR::wzr)));
                nodeToVReg_[node] = dstReg;
                return dstReg;
            }

            int      bits  = FLOAT_TO_INT_BITS(fval);
            Register wTmp  = getVReg(BE::I32);
            auto     segs  = decomposeImm64(static_cast<unsigned long long>(static_cast<uint32_t>(bits)));
            bool     first = true;
            for (size_t i = 0; i < 2; ++i)
            {
                if (segs[i] == 0 && !first) continue;
                if (first)
                {
                    m_block->insts.push_back(
                        createInstr2(Operator::MOVZ, new RegOperand(wTmp), new ImmeOperand(segs[i])));
                    first = false;
                }
                else
                {
                    m_block->insts.push_back(createInstr3(
                        Operator::MOVK, new RegOperand(wTmp), new ImmeOperand(segs[i]), new ImmeOperand(i * 16)));
                }
            }
            m_block->insts.push_back(createInstr2(Operator::FMOV, new RegOperand(dstReg), new RegOperand(wTmp)));

            nodeToVReg_[node] = dstReg;
            return dstReg;
        }

        if (opcode == DAG::ISD::FRAME_INDEX || opcode == DAG::ISD::SYMBOL) return materializeAddress(node, m_block);

        ERROR("Node not scheduled or cannot be materialized: %s",
            DAG::toString(static_cast<DAG::ISD>(node->getOpcode())));
        return Register();
    }

    Register DAGIsel::materializeAddress(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // 地址实例化：使用者负责实例化
        // ============================================================================
        //
        // 作用：将地址节点（FRAME_INDEX/SYMBOL）实例化为寄存器
        //
        // 为什么需要：
        // - 地址节点本身不生成独立的指令，而是由使用者（LOAD/STORE/ADD）决定如何实例化
        // - FRAME_INDEX 需要生成抽象的 FrameIndexOperand，由后续 Pass 替换为实际偏移
        // - SYMBOL 需要生成 LA 伪指令加载全局变量地址

        if (!node) ERROR("Cannot materialize null address");

        auto opcode = static_cast<DAG::ISD>(node->getOpcode());

        if (opcode == DAG::ISD::FRAME_INDEX && node->hasIRRegId())
        {
            size_t   ir_reg_id = node->getIRRegId();
            Register addrReg   = getVReg(BE::I64);

            Instr* addr_inst     = createInstr2(Operator::ADD, new RegOperand(addrReg), new RegOperand(PR::sp));
            addr_inst->fiop      = new FrameIndexOperand(ir_reg_id);
            addr_inst->use_fiops = true;
            m_block->insts.push_back(addr_inst);

            return addrReg;
        }

        if (opcode == DAG::ISD::SYMBOL && node->hasSymbol())
        {
            Register addrReg = getVReg(BE::I64);
            m_block->insts.push_back(
                createInstr2(Operator::LA, new RegOperand(addrReg), new SymbolOperand(node->getSymbol())));
            return addrReg;
        }

        auto it = nodeToVReg_.find(node);
        if (it != nodeToVReg_.end()) return it->second;

        if (opcode == DAG::ISD::REG && node->hasIRRegId())
            return getOrCreateVReg(node->getIRRegId(), node->getNumValues() > 0 ? node->getValueType(0) : BE::I64);

        ERROR("Cannot materialize address for opcode: %s", DAG::toString(opcode));
    }

    int DAGIsel::dataTypeSize(BE::DataType* dt)
    {
        if (dt == BE::I32 || dt == BE::F32) return 4;
        if (dt == BE::I64 || dt == BE::F64 || dt == BE::PTR) return 8;
        return 4;
    }

    Register DAGIsel::getOrCreateVReg(size_t ir_reg_id, BE::DataType* dt)
    {
        auto it = ctx_.vregMap.find(ir_reg_id);
        if (it != ctx_.vregMap.end()) return it->second;

        Register vreg           = getVReg(dt);
        ctx_.vregMap[ir_reg_id] = vreg;
        return vreg;
    }

    void DAGIsel::importGlobals()
    {
        // ============================================================================
        // TODO: 导入全局变量
        // ============================================================================
        //
        // 作用：
        // 将 IR 模块中的全局变量转换为后端的 GlobalVariable 对象。
        //
        // 为什么需要：
        // - 全局变量需在生成的汇编中声明和初始化
        // - 需转换类型（ME::DataType → BE::DataType）
        // - 需处理初始化值（标量 vs 数组）
        //
        // 关键点：
        // - 遍历 ir_module_->globalVars
        // - 创建 BE::GlobalVariable 对象
        // - 处理数组维度（arrayDims）与初始值（initList/init）
        // - 浮点数需位转换（FLOAT_TO_INT_BITS）

        TODO("导入全局变量到后端模块");
    }

    void DAGIsel::collectAllocas(ME::Function* ir_func)
    {
        // ============================================================================
        // TODO: 收集函数中的所有 alloca 并注册到 frameInfo
        // ============================================================================
        //
        // 作用：
        // 遍历函数的所有 IR 指令，找到所有 ALLOCA 指令，计算其需要的栈空间大小，
        // 并在函数级别的栈帧管理中为其分配槽位。

        TODO("收集 alloca 并注册到 frameInfo");
    }

    void DAGIsel::setupParameters(ME::Function* ir_func)
    {
        // ============================================================================
        // TODO: 为函数参数创建虚拟寄存器
        // ============================================================================
        //
        // 作用：
        // 遍历 IR 函数定义中的所有参数，为每个参数分配虚拟寄存器，并记录参数的虚拟寄存器映射关系

        TODO("为函数参数创建虚拟寄存器并建立映射");
    }

    bool DAGIsel::selectAddress(const DAG::SDNode* addrNode, const DAG::SDNode*& baseNode, int64_t& offset)
    {
        if (!addrNode) return false;

        auto opcode = static_cast<DAG::ISD>(addrNode->getOpcode());

        if (opcode == DAG::ISD::FRAME_INDEX || opcode == DAG::ISD::SYMBOL)
        {
            baseNode = addrNode;
            offset   = 0;
            return true;
        }

        if (opcode == DAG::ISD::ADD)
        {
            const DAG::SDNode* lhs = addrNode->getOperand(0).getNode();
            const DAG::SDNode* rhs = addrNode->getOperand(1).getNode();

            const DAG::SDNode* lhsBase;
            int64_t            lhsOffset = 0;
            if (selectAddress(lhs, lhsBase, lhsOffset))
            {
                if ((static_cast<DAG::ISD>(rhs->getOpcode()) == DAG::ISD::CONST_I32 ||
                        static_cast<DAG::ISD>(rhs->getOpcode()) == DAG::ISD::CONST_I64) &&
                    rhs->hasImmI64())
                {
                    baseNode = lhsBase;
                    offset   = lhsOffset + rhs->getImmI64();
                    return true;
                }
                return false;
            }

            const DAG::SDNode* rhsBase;
            int64_t            rhsOffset = 0;
            if (selectAddress(rhs, rhsBase, rhsOffset))
            {
                if ((static_cast<DAG::ISD>(lhs->getOpcode()) == DAG::ISD::CONST_I32 ||
                        static_cast<DAG::ISD>(lhs->getOpcode()) == DAG::ISD::CONST_I64) &&
                    lhs->hasImmI64())
                {
                    baseNode = rhsBase;
                    offset   = rhsOffset + lhs->getImmI64();
                    return true;
                }
                return false;
            }

            return false;
        }

        return false;
    }

    void DAGIsel::selectCopy(const DAG::SDNode* node, BE::Block* m_block)
    {
        if (node->getNumOperands() < 1) return;

        const DAG::SDNode* src = node->getOperand(0).getNode();
        if (!src) return;

        Register dst    = getOperandReg(node, m_block);
        Register srcReg = getOperandReg(src, m_block);

        m_block->insts.push_back(BE::AArch64::createMove(new RegOperand(dst), new RegOperand(srcReg), LOC_STR));
    }

    void DAGIsel::selectConst(const DAG::SDNode* node, BE::Block* m_block) { TODO("选择常量节点"); }

    void DAGIsel::selectPhi(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择 PHI 节点
        // ============================================================================
        //
        // 作用：
        // 为 PHI 节点生成 MIR 的 PhiInst，记录所有前驱块与对应的值。
        //
        // 为什么需要：
        // - PHI 节点在 SSA 中合并来自不同前驱的值
        // - 需要保留前驱块信息，供后续 PHI 消解 Pass 使用
        //
        // 关键点：
        // - PHI 操作数成对出现：[label0, val0, label1, val1, ...]
        // - 常量可直接作为立即数，无需实例化为寄存器

        TODO("选择 PHI 节点：提取前驱与值，生成 PhiInst");
    }

    void DAGIsel::selectBinary(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // 选择二元运算节点（示例实现，保留）
        // ============================================================================
        //
        // AArch64 特殊处理：
        // - 寄存器位宽匹配：x/w 寄存器别名需要类型转换
        // - MOD 运算：展开为 a - (a / b) * b

        if (node->getNumOperands() < 2) return;

        auto opcode = static_cast<DAG::ISD>(node->getOpcode());

        Register dst = nodeToVReg_.at(node);

        const DAG::SDNode* lhs = node->getOperand(0).getNode();
        const DAG::SDNode* rhs = node->getOperand(1).getNode();

        Register lhsReg = getOperandReg(lhs, m_block);
        Register rhsReg = getOperandReg(rhs, m_block);

        Operator op;
        bool     isFloat = (dst.dt == BE::F32 || dst.dt == BE::F64);

        switch (opcode)
        {
            case DAG::ISD::ADD: op = isFloat ? Operator::FADD : Operator::ADD; break;
            case DAG::ISD::SUB: op = isFloat ? Operator::FSUB : Operator::SUB; break;
            case DAG::ISD::MUL: op = isFloat ? Operator::FMUL : Operator::MUL; break;
            case DAG::ISD::DIV: op = isFloat ? Operator::FDIV : Operator::SDIV; break;
            case DAG::ISD::FADD: op = Operator::FADD; break;
            case DAG::ISD::FSUB: op = Operator::FSUB; break;
            case DAG::ISD::FMUL: op = Operator::FMUL; break;
            case DAG::ISD::FDIV: op = Operator::FDIV; break;
            case DAG::ISD::AND: op = Operator::AND; break;
            case DAG::ISD::OR: op = Operator::ORR; break;
            case DAG::ISD::XOR: op = Operator::EOR; break;
            case DAG::ISD::SHL: op = Operator::LSL; break;
            case DAG::ISD::ASHR: op = Operator::ASR; break;
            case DAG::ISD::LSHR: op = Operator::LSR; break;
            case DAG::ISD::MOD: ERROR("MOD operator is not supported for AArch64. How to handle this?"); return;
            default: ERROR("Unsupported binary operator: %d", node->getOpcode()); return;
        }

        m_block->insts.push_back(createInstr3(op, new RegOperand(dst), new RegOperand(lhsReg), new RegOperand(rhsReg)));
    }

    void DAGIsel::selectUnary(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择一元运算节点，但也许不用做任何事情
        // ============================================================================

        TODO("选择一元运算节点");
    }

    void DAGIsel::selectLoad(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // 选择 LOAD 节点, 作为示例实现，仅供参考
        // ============================================================================
        //
        // 作用：
        // 为 LOAD 节点生成目标相关的访存指令（LDR）。
        //
        // 为什么需要地址选择：
        // - 地址可能是简单的 [base + offset]，也可能是复杂表达式
        // - 声明式地址选择（selectAddress）可将常见模式折叠到访存指令的立即数字段
        // - 若地址过于复杂，需先完整计算到寄存器

        if (node->getNumOperands() < 2) return;

        Register           dst  = nodeToVReg_.at(node);
        const DAG::SDNode* addr = node->getOperand(1).getNode();

        const DAG::SDNode* baseNode;
        int64_t            offset = 0;

        if (selectAddress(addr, baseNode, offset))
        {
            Register baseReg;

            if (static_cast<DAG::ISD>(baseNode->getOpcode()) == DAG::ISD::FRAME_INDEX)
            {
                int fi              = baseNode->getFrameIndex();
                baseReg             = getVReg(BE::I64);
                Instr* addrInst     = createInstr2(Operator::ADD, new RegOperand(baseReg), new RegOperand(PR::sp));
                addrInst->fiop      = new FrameIndexOperand(fi);
                addrInst->use_fiops = true;
                m_block->insts.push_back(addrInst);
            }
            else if (static_cast<DAG::ISD>(baseNode->getOpcode()) == DAG::ISD::SYMBOL && baseNode->hasSymbol())
            {
                std::string symbol = baseNode->getSymbol();
                baseReg            = getVReg(BE::I64);
                m_block->insts.push_back(
                    createInstr2(Operator::LA, new RegOperand(baseReg), new SymbolOperand(symbol)));
            }
            else { baseReg = getOperandReg(baseNode, m_block); }

            m_block->insts.push_back(
                createInstr2(Operator::LDR, new RegOperand(dst), new MemOperand(baseReg, static_cast<int>(offset))));
        }
        else
        {
            Register addrReg = getOperandReg(addr, m_block);
            m_block->insts.push_back(createInstr2(Operator::LDR, new RegOperand(dst), new MemOperand(addrReg, 0)));
        }
    }

    void DAGIsel::selectStore(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择 STORE 节点
        // ============================================================================
        //
        // 作用：
        // 为 STORE 节点生成目标相关的存储指令（STR）。
        //
        // 与 LOAD 相似：
        // - 同样需要地址选择与立即数范围检查
        // - 操作数：[Chain, Value, Address]

        TODO("选择 STORE：地址折叠 + 生成存储指令");
    }

    void DAGIsel::selectICmp(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择整数比较节点（ICMP）
        // ============================================================================

        TODO("选择 ICMP：根据条件生成比较指令序列");
    }

    void DAGIsel::selectFCmp(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择浮点比较节点（FCMP）
        // ============================================================================

        TODO("选择 FCMP：根据条件生成浮点比较指令");
    }

    void DAGIsel::selectBranch(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择分支节点（BR / BRCOND）
        // ============================================================================
        //
        // 作用：
        // 为无条件分支与条件分支生成跳转指令。
        // - BR 直接跳转，用 B label
        // - BRCOND 需判断条件（非 0 为真），生成 CMP + BNE + B 序列

        TODO("选择分支：区分 BR/BRCOND 生成跳转指令");
    }

    void DAGIsel::selectCall(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择 CALL 节点
        // ============================================================================
        //
        // 作用：
        // 为函数调用生成参数传递、CALL 指令、返回值处理的完整序列。
        // - 需要遵循目标调用约定（参数寄存器 vs 栈传参）
        // - 整数与浮点参数使用不同的寄存器组
        // - 内置函数（llvm.memset 等）需特殊处理，转化为对 memset 的调用
        //
        // AArch64 调用约定：
        // - 整数参数：x0-x7 (w0-w7)
        // - 浮点参数：d0-d7 (s0-s7)
        // - 超出的参数 → 存储到栈上（SP + 0, SP + 8, ...）
        // - 返回值从 x0/w0/s0/d0 搬运到目标寄存器

        TODO("选择 CALL：参数传递 + 生成调用指令 + 返回值处理");
    }

    void DAGIsel::selectRet(const DAG::SDNode* node, BE::Block* m_block)
    {
        // 操作数 0 是 Chain（保证副作用顺序），操作数 1 是实际返回值
        // 如有返回值，则将返回值移动到 x0/w0/s0/d0
        if (node->getNumOperands() > 1)
        {
            const DAG::SDNode* retValNode = node->getOperand(1).getNode();
            Register           retReg     = getOperandReg(retValNode, m_block);

            DataType* retType = retValNode->getNumValues() > 0 ? retValNode->getValueType(0) : BE::I32;

            Register destReg;
            if (retType == BE::F32 || retType == BE::F64)
                destReg = (retType == BE::F32) ? PR::s0 : PR::d0;
            else
                destReg = (retType == BE::I32) ? PR::w0 : PR::x0;

            if (retReg.dt->equal(I64) && destReg.dt->equal(I32)) destReg.dt = I64;
            m_block->insts.push_back(BE::AArch64::createMove(new RegOperand(destReg), new RegOperand(retReg), LOC_STR));
        }

        m_block->insts.push_back(createInstr0(Operator::RET));
    }

    void DAGIsel::selectCast(const DAG::SDNode* node, BE::Block* m_block)
    {
        // ============================================================================
        // TODO: 选择类型转换节点（ZEXT / SITOFP / FPTOSI）
        // ============================================================================
        //
        // 作用：
        // 为类型转换节点生成目标相关的转换指令。

        TODO("选择类型转换：生成 UXTW/SCVTF/FCVTZS 指令");
    }

    void DAGIsel::selectNode(const DAG::SDNode* node, BE::Block* m_block)
    {
        if (!node) return;

        auto opcode = static_cast<DAG::ISD>(node->getOpcode());

        switch (opcode)
        {
            case DAG::ISD::LABEL:
            case DAG::ISD::SYMBOL:
            case DAG::ISD::ENTRY_TOKEN:
            case DAG::ISD::TOKEN_FACTOR:
            case DAG::ISD::FRAME_INDEX:
            case DAG::ISD::REG: break;
            case DAG::ISD::COPY: selectCopy(node, m_block); break;
            case DAG::ISD::PHI: selectPhi(node, m_block); break;
            case DAG::ISD::CONST_I32:
            case DAG::ISD::CONST_I64:
            case DAG::ISD::CONST_F32: selectConst(node, m_block); break;
            case DAG::ISD::ADD:
            case DAG::ISD::SUB:
            case DAG::ISD::MUL:
            case DAG::ISD::DIV:
            case DAG::ISD::MOD:
            case DAG::ISD::AND:
            case DAG::ISD::OR:
            case DAG::ISD::XOR:
            case DAG::ISD::SHL:
            case DAG::ISD::ASHR:
            case DAG::ISD::LSHR:
            case DAG::ISD::FADD:
            case DAG::ISD::FSUB:
            case DAG::ISD::FMUL:
            case DAG::ISD::FDIV: selectBinary(node, m_block); break;
            case DAG::ISD::LOAD: selectLoad(node, m_block); break;
            case DAG::ISD::STORE: selectStore(node, m_block); break;
            case DAG::ISD::ICMP: selectICmp(node, m_block); break;
            case DAG::ISD::FCMP: selectFCmp(node, m_block); break;
            case DAG::ISD::BR:
            case DAG::ISD::BRCOND: selectBranch(node, m_block); break;
            case DAG::ISD::CALL: selectCall(node, m_block); break;
            case DAG::ISD::RET: selectRet(node, m_block); break;
            case DAG::ISD::ZEXT:
            case DAG::ISD::SITOFP:
            case DAG::ISD::FPTOSI: selectCast(node, m_block); break;
            default: ERROR("Unsupported DAG node: %s", DAG::toString(static_cast<DAG::ISD>(node->getOpcode())));
        }
    }

    void DAGIsel::selectBlock(ME::Block* ir_block, const DAG::SelectionDAG& dag)
    {
        // ============================================================================
        // TODO: 选择基本块的所有指令
        // ============================================================================
        //
        // 作用：
        // 将一个基本块的 DAG 转换为 MIR 指令序列。
        //
        // 采用两阶段：
        // - 阶段 1：调度与寄存器分配
        //   * 调度 DAG 节点，确定指令顺序
        //   * 为每个节点预分配虚拟寄存器（如果不预分配会怎么样？可以怎么解决？）
        // - 阶段 2：指令选择
        //   * 按调度顺序遍历节点，调用 selectNode 生成具体指令
        //   * 使用已选择集合避免重复选择

        TODO("选择基本块：调度 + 分配寄存器 + 生成指令");
    }

    void DAGIsel::selectFunction(ME::Function* ir_func)
    {
        // ============================================================================
        // TODO: 选择函数的所有指令
        // ============================================================================
        //
        // 作用：
        // 协调整个函数的指令选择流程，包括栈帧管理、参数设置、基本块选择。
        //
        // 为什么需要函数级初始化：
        // - 每个函数有独立的虚拟寄存器空间（vregMap、nodeToVReg_）
        // - 需要计算栈帧布局（传出参数区、局部变量区）
        // - 需要创建所有基本块的 MIR 对象
        //
        // 关键步骤：
        // 1. 重置上下文
        // 2. 创建后端函数对象
        // 3. 计算传出参数区大小
        // 4. 收集局部变量
        // 5. 创建所有基本块对象
        // 6. 为参数分配虚拟寄存器
        // 7. 对每个基本块做指令选择

        // Hint: 建议了解下对于函数来说，哪些寄存器是由调用者保存，哪些是被调用者保存的

        TODO("选择函数：初始化上下文 + 栈帧管理 + 逐块选择");
    }

    void DAGIsel::runImpl()
    {
        importGlobals();

        target_->buildDAG(ir_module_);

        for (auto* f : ir_module_->functions) selectFunction(f);
    }

}  // namespace BE::AArch64
