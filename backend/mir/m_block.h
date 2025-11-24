#ifndef __BACKEND_MIR_BLOCK_H__
#define __BACKEND_MIR_BLOCK_H__

#include "middleend/module/ir_block.h"
#include <backend/mir/m_instruction.h>
#include <deque>

namespace BE
{
    class Block
    {
      public:
        std::deque<MInstruction*> insts;
        uint32_t                  blockId;

      public:
        Block(uint32_t id) : blockId(id) {}
        ~Block()
        {
            for (auto inst : insts) MInstruction::delInst(inst);
            insts.clear();
        }
    };
}  // namespace BE

#endif  // __BACKEND_MIR_BLOCK_H__
