#ifndef __BACKEND_DAG_FOLDING_SET_H__
#define __BACKEND_DAG_FOLDING_SET_H__

#include <cstdint>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <string>

namespace BE
{
    namespace DAG
    {
        class FoldingSetNodeID
        {
            std::vector<uint32_t> bits_;

          public:
            FoldingSetNodeID() = default;

            void AddInteger(int64_t value)
            {
                bits_.push_back(static_cast<uint32_t>(value & 0xFFFFFFFF));
                bits_.push_back(static_cast<uint32_t>((value >> 32) & 0xFFFFFFFF));
            }

            void AddPointer(const void* ptr)
            {
                uintptr_t val = reinterpret_cast<uintptr_t>(ptr);
                bits_.push_back(static_cast<uint32_t>(val & 0xFFFFFFFF));
                if (sizeof(void*) > 4) { bits_.push_back(static_cast<uint32_t>((val >> 32) & 0xFFFFFFFF)); }
            }

            void AddString(const std::string& str)
            {
                bits_.push_back(static_cast<uint32_t>(str.size()));
                const char* data   = str.data();
                size_t      len    = str.size();
                size_t      offset = 0;
                while (offset < len)
                {
                    uint32_t chunk      = 0;
                    size_t   chunk_size = std::min<size_t>(4, len - offset);
                    std::memcpy(&chunk, data + offset, chunk_size);
                    bits_.push_back(chunk);
                    offset += chunk_size;
                }
            }

            void AddFloat(float value)
            {
                uint32_t bits;
                std::memcpy(&bits, &value, sizeof(float));
                bits_.push_back(bits);
            }

            void AddBoolean(bool value) { bits_.push_back(value ? 1 : 0); }

            uint64_t computeHash() const
            {
                // FNV-1a hash
                uint64_t hash = 14695981039346656037ULL;
                for (uint32_t bit : bits_)
                {
                    hash ^= bit;
                    hash *= 1099511628211ULL;
                }
                return hash;
            }

            bool operator==(const FoldingSetNodeID& other) const { return bits_ == other.bits_; }
            bool operator!=(const FoldingSetNodeID& other) const { return bits_ != other.bits_; }
        };

    }  // namespace DAG
}  // namespace BE

namespace std
{
    template <>
    struct hash<BE::DAG::FoldingSetNodeID>
    {
        size_t operator()(const BE::DAG::FoldingSetNodeID& id) const { return id.computeHash(); }
    };
}  // namespace std

#endif  // __BACKEND_DAG_FOLDING_SET_H__
