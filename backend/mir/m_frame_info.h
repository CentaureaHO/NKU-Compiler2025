#ifndef __BACKEND_MIR_M_FRAME_INFO_H__
#define __BACKEND_MIR_M_FRAME_INFO_H__

#include <unordered_map>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace BE
{
    class MFrameInfo
    {
      public:
        enum class ObjectKind
        {
            LocalVar,   // alloca
            SpillSlot,  // RA spill
            OutArg      // outgoing call args
        };

        struct FrameObject
        {
            int        size      = 0;   // bytes
            int        alignment = 16;  // bytes
            int        offset    = -1;  // bytes from SP after prologue (>=0), assigned by calculateOffsets()
            ObjectKind kind      = ObjectKind::LocalVar;
        };

      private:
        std::unordered_map<size_t, FrameObject> irRegToObject_;   // IR reg id -> object (for alloca)
        std::vector<FrameObject>                spillSlots_;      // spill slots (indexed by FI)
        int                                     paramSize_ = 0;   // outgoing args area (max)
        int                                     baseAlign_ = 16;  // default stack alignment

        static inline int alignTo(int v, int a) { return (v + (a - 1)) & ~(a - 1); }

      public:
        MFrameInfo() = default;

        void clear()
        {
            irRegToObject_.clear();
            spillSlots_.clear();
            paramSize_ = 0;
        }

        void createLocalObject(size_t irRegId, int sizeBytes, int alignment = 16)
        {
            irRegToObject_[irRegId] = FrameObject{sizeBytes, std::max(16, alignment), -1, ObjectKind::LocalVar};
        }

        int createSpillSlot(int sizeBytes, int alignment = 8)
        {
            int fi = static_cast<int>(spillSlots_.size());
            spillSlots_.push_back(FrameObject{sizeBytes, std::max(8, alignment), -1, ObjectKind::SpillSlot});
            return fi;
        }

        int getObjectOffset(size_t irRegId) const
        {
            auto it = irRegToObject_.find(irRegId);
            if (it == irRegToObject_.end()) return -1;
            return it->second.offset;
        }

        int getSpillSlotOffset(int fi) const
        {
            if (fi < 0 || fi >= static_cast<int>(spillSlots_.size())) return -1;
            return spillSlots_[fi].offset;
        }

        bool hasObject(size_t irRegId) const { return irRegToObject_.find(irRegId) != irRegToObject_.end(); }

        void setParamAreaSize(int bytes) { paramSize_ = std::max(paramSize_, alignTo(bytes, 16)); }
        int  getParamAreaSize() const { return paramSize_; }

        void setBaseAlignment(int a) { baseAlign_ = std::max(8, a); }
        int  getBaseAlignment() const { return baseAlign_; }

        int calculateOffsets()
        {
            int currentOffset = paramSize_;

            for (auto& [irRegId, obj] : irRegToObject_)
            {
                currentOffset = alignTo(currentOffset, obj.alignment);
                obj.offset    = currentOffset;
                currentOffset += obj.size;
            }

            for (auto& slot : spillSlots_)
            {
                currentOffset = alignTo(currentOffset, slot.alignment);
                slot.offset   = currentOffset;
                currentOffset += slot.size;
            }

            return alignTo(currentOffset, baseAlign_);
        }

        int getStackSize() const
        {
            int maxOff = paramSize_;
            for (auto& [_, obj] : irRegToObject_)
                if (obj.offset >= 0) maxOff = std::max(maxOff, obj.offset + obj.size);
            for (auto& slot : spillSlots_)
                if (slot.offset >= 0) maxOff = std::max(maxOff, slot.offset + slot.size);
            return alignTo(maxOff, baseAlign_);
        }

        int createOrGetObject(size_t irRegId, int sizeBytes, int alignment = 16)
        {
            auto it = irRegToObject_.find(irRegId);
            if (it != irRegToObject_.end() && it->second.offset >= 0) return it->second.offset;

            createLocalObject(irRegId, sizeBytes, alignment);
            int currentOffset = paramSize_;
            for (auto& [id, obj] : irRegToObject_)
                if (obj.offset >= 0) currentOffset = std::max(currentOffset, obj.offset + obj.size);

            currentOffset                  = alignTo(currentOffset, alignment);
            irRegToObject_[irRegId].offset = currentOffset;
            return currentOffset;
        }
    };
}  // namespace BE

#endif  // __BACKEND_MIR_M_FRAME_INFO_H__
