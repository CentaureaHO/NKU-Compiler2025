#include <dynamic_bitset.h>
#include <algorithm>
#include <cstring>
#include <cmath>

#ifdef __GNUC__
#include <x86intrin.h>
#define CELE_ALIGNED(x) __attribute__((aligned(x)))
#else
#define CELE_ALIGNED(x) __declspec(align(x))
#endif

static constexpr size_t log2_constexpr(size_t n) { return (n <= 1) ? 0 : 1 + log2_constexpr(n >> 1); }

static constexpr size_t BLOCK_SHIFT = log2_constexpr(dynamic_bitset::bits_per_block);
static constexpr size_t BLOCK_MASK  = dynamic_bitset::bits_per_block - 1;

static_assert((dynamic_bitset::bits_per_block & (dynamic_bitset::bits_per_block - 1)) == 0,
    "bits_per_block must be a power of 2 for bit-shift optimizations");

size_t dynamic_bitset::popcount(block_type block)
{
#if defined(__GNUC__) && defined(__POPCNT__)
    return __builtin_popcountl(block);
#elif defined(__GNUC__)
    return __builtin_popcountl(block);
#else

    size_t count = 0;
    while (block)
    {
        block &= block - 1;
        ++count;
    }
    return count;
#endif
}

void dynamic_bitset::sanitize()
{
    if (m_num_blocks > 0)
    {
        size_t extra_bits = unused_bits();
        if (extra_bits > 0) m_blocks[m_num_blocks - 1] &= ~(block_type(~0) << (bits_per_block - extra_bits));
    }
}

dynamic_bitset::dynamic_bitset() : m_blocks(nullptr), m_num_bits(0), m_num_blocks(0) {}

dynamic_bitset::dynamic_bitset(size_t num_bits, unsigned long value)
    : m_num_bits(num_bits), m_num_blocks(blocks_required(num_bits))
{
    if (m_num_blocks > 0)
    {
#if defined(__GNUC__) && !defined(__MINGW32__)
        const size_t alignment = 64;
        void*        ptr       = nullptr;
        if (posix_memalign(&ptr, alignment, m_num_blocks * sizeof(block_type)) == 0)
        {
            m_blocks = static_cast<block_type*>(ptr);
            std::memset(m_blocks, 0, m_num_blocks * sizeof(block_type));
        }
        else
            m_blocks = new block_type[m_num_blocks]();
#else
        m_blocks = new block_type[m_num_blocks]();
#endif

        if (value != 0)
        {
            m_blocks[0] = value;
            sanitize();
        }
    }
    else
        m_blocks = nullptr;
}

dynamic_bitset::dynamic_bitset(const std::string& str, size_t pos, size_t n, char zero, char one)
    : m_blocks(nullptr), m_num_bits(0), m_num_blocks(0)
{
    if (pos >= str.size()) return;

    if (n == std::string::npos) n = str.size() - pos;

    m_num_bits   = n;
    m_num_blocks = blocks_required(m_num_bits);

    if (m_num_blocks > 0)
    {
        m_blocks = new block_type[m_num_blocks]();

        for (size_t i = 0; i < n; ++i)
        {
            char c = str[pos + i];
            if (c == one)
            {
                size_t bit_pos    = n - i - 1;
                size_t block_idx  = bit_pos / bits_per_block;
                size_t bit_offset = bit_pos % bits_per_block;
                m_blocks[block_idx] |= (block_type(1) << bit_offset);
            }
            else if (c != zero)
                throw std::invalid_argument("Invalid character in bitset initialization");
        }
    }
}

dynamic_bitset::dynamic_bitset(const dynamic_bitset& other)
    : m_num_bits(other.m_num_bits), m_num_blocks(other.m_num_blocks)
{
    if (m_num_blocks > 0)
    {
#if defined(__GNUC__) && !defined(__MINGW32__)
        const size_t alignment = 64;
        void*        ptr       = nullptr;
        if (posix_memalign(&ptr, alignment, m_num_blocks * sizeof(block_type)) == 0)
            m_blocks = static_cast<block_type*>(ptr);
        else
            m_blocks = new block_type[m_num_blocks];
#else
        m_blocks = new block_type[m_num_blocks];
#endif

        std::memcpy(m_blocks, other.m_blocks, m_num_blocks * sizeof(block_type));
    }
    else { m_blocks = nullptr; }
}

dynamic_bitset::dynamic_bitset(dynamic_bitset&& other) noexcept
    : m_blocks(other.m_blocks), m_num_bits(other.m_num_bits), m_num_blocks(other.m_num_blocks)
{
    other.m_blocks     = nullptr;
    other.m_num_bits   = 0;
    other.m_num_blocks = 0;
}

dynamic_bitset& dynamic_bitset::operator=(const dynamic_bitset& other)
{
    if (this != &other)
    {
        if (m_num_blocks != other.m_num_blocks)
        {
#if defined(__GNUC__) && !defined(__MINGW32__)
            if (m_blocks) free(m_blocks);
#else
            delete[] m_blocks;
#endif

            m_num_blocks = other.m_num_blocks;

            if (m_num_blocks > 0)
            {
#if defined(__GNUC__) && !defined(__MINGW32__)
                const size_t alignment = 64;
                void*        ptr       = nullptr;
                if (posix_memalign(&ptr, alignment, m_num_blocks * sizeof(block_type)) == 0)
                    m_blocks = static_cast<block_type*>(ptr);
                else
                    m_blocks = new block_type[m_num_blocks];
#else
                m_blocks = new block_type[m_num_blocks];
#endif
            }
            else
                m_blocks = nullptr;
        }

        m_num_bits = other.m_num_bits;

        if (m_num_blocks > 0) std::memcpy(m_blocks, other.m_blocks, m_num_blocks * sizeof(block_type));
    }

    return *this;
}

dynamic_bitset& dynamic_bitset::operator=(dynamic_bitset&& other) noexcept
{
    if (this != &other)
    {
        // 使用与析构函数匹配的释放方式
#if defined(__GNUC__) && !defined(__MINGW32__)
        if (m_blocks) free(m_blocks);
#else
        delete[] m_blocks;
#endif

        m_blocks     = other.m_blocks;
        m_num_bits   = other.m_num_bits;
        m_num_blocks = other.m_num_blocks;

        other.m_blocks     = nullptr;
        other.m_num_bits   = 0;
        other.m_num_blocks = 0;
    }
    return *this;
}

dynamic_bitset::~dynamic_bitset()
{
#if defined(__GNUC__) && !defined(__MINGW32__)
    if (m_blocks)
    {
        free(m_blocks);
        m_blocks = nullptr;
    }
#else
    delete[] m_blocks;
#endif
}

dynamic_bitset& dynamic_bitset::set(size_t pos, bool value)
{
#ifndef NDEBUG
    check_bounds(pos);
#endif

    const size_t block = pos >> BLOCK_SHIFT;
    const size_t bit   = pos & BLOCK_MASK;

    m_blocks[block] = (m_blocks[block] & ~(block_type(1) << bit)) | (block_type(value) << bit);

    return *this;
}

dynamic_bitset& dynamic_bitset::set_range(size_t start, size_t count, bool value)
{
    if (start + count > m_num_bits) throw std::out_of_range("Range exceeds bitset size");

    if (count == 0) return *this;

    size_t first_block = start / bits_per_block;
    size_t last_block  = (start + count - 1) / bits_per_block;

    size_t first_bit = start % bits_per_block;
    size_t last_bit  = (start + count - 1) % bits_per_block;

    if (first_block == last_block)
    {
        block_type mask = ((block_type(1) << (last_bit - first_bit + 1)) - 1) << first_bit;

        if (value)
            m_blocks[first_block] |= mask;
        else
            m_blocks[first_block] &= ~mask;
    }
    else
    {
        block_type first_mask = ~block_type(0) << first_bit;
        if (value)
            m_blocks[first_block] |= first_mask;
        else
            m_blocks[first_block] &= ~first_mask;

        block_type fill_value = value ? ~block_type(0) : block_type(0);
        for (size_t i = first_block + 1; i < last_block; ++i) m_blocks[i] = fill_value;

        block_type last_mask = (block_type(1) << (last_bit + 1)) - 1;
        if (value)
            m_blocks[last_block] |= last_mask;
        else
            m_blocks[last_block] &= ~last_mask;
    }

    return *this;
}

dynamic_bitset& dynamic_bitset::reset(size_t pos) { return set(pos, false); }

dynamic_bitset& dynamic_bitset::flip(size_t pos)
{
#ifndef NDEBUG
    check_bounds(pos);
#endif

    const size_t block = pos >> BLOCK_SHIFT;
    const size_t bit   = pos & BLOCK_MASK;

    m_blocks[block] ^= (block_type(1) << bit);

    return *this;
}

dynamic_bitset& dynamic_bitset::set()
{
    if (m_num_blocks > 0)
    {
        std::memset(m_blocks, 0xFF, m_num_blocks * sizeof(block_type));
        sanitize();
    }
    return *this;
}

dynamic_bitset& dynamic_bitset::reset()
{
    if (m_num_blocks > 0) std::memset(m_blocks, 0, m_num_blocks * sizeof(block_type));
    return *this;
}

dynamic_bitset& dynamic_bitset::flip()
{
    for (size_t i = 0; i < m_num_blocks; ++i) m_blocks[i] = ~m_blocks[i];
    sanitize();
    return *this;
}

bool dynamic_bitset::test(size_t pos) const
{
#ifndef NDEBUG
    check_bounds(pos);
#endif

    const size_t block = pos >> BLOCK_SHIFT;
    const size_t bit   = pos & BLOCK_MASK;

    return (m_blocks[block] & (block_type(1) << bit)) != 0;
}

bool dynamic_bitset::operator[](size_t pos) const
{
    if (pos >= m_num_bits) return false;

    const size_t block = pos >> BLOCK_SHIFT;
    const size_t bit   = pos & BLOCK_MASK;

    return (m_blocks[block] & (block_type(1) << bit)) != 0;
}

size_t dynamic_bitset::size() const { return m_num_bits; }

void dynamic_bitset::resize(size_t num_bits, bool value)
{
    if (num_bits == m_num_bits) return;

    size_t new_num_blocks = blocks_required(num_bits);

    if (new_num_blocks != m_num_blocks)
    {
        block_type* new_blocks = nullptr;

        if (new_num_blocks > 0)
        {
#if defined(__GNUC__) && !defined(__MINGW32__)
            const size_t alignment = 64;
            void*        ptr       = nullptr;
            if (posix_memalign(&ptr, alignment, new_num_blocks * sizeof(block_type)) == 0)
            {
                new_blocks = static_cast<block_type*>(ptr);
                std::memset(new_blocks, 0, new_num_blocks * sizeof(block_type));
            }
            else
                new_blocks = new block_type[new_num_blocks]();
#else
            new_blocks = new block_type[new_num_blocks]();
#endif
        }

        size_t copy_blocks = std::min(m_num_blocks, new_num_blocks);
        if (copy_blocks > 0) std::memcpy(new_blocks, m_blocks, copy_blocks * sizeof(block_type));

        if (value && new_num_blocks > m_num_blocks)
            std::fill(new_blocks + m_num_blocks, new_blocks + new_num_blocks, ~block_type(0));
        else if (new_num_blocks > m_num_blocks)
            std::fill(new_blocks + m_num_blocks, new_blocks + new_num_blocks, block_type(0));

            // 使用与析构函数匹配的释放方式
#if defined(__GNUC__) && !defined(__MINGW32__)
        if (m_blocks) free(m_blocks);
#else
        delete[] m_blocks;
#endif
        m_blocks     = new_blocks;
        m_num_blocks = new_num_blocks;
    }
    else if (num_bits > m_num_bits && value)
    {
        size_t old_bits_in_last_block = m_num_bits % bits_per_block;

        if (old_bits_in_last_block > 0 && m_num_blocks > 0)
            m_blocks[m_num_blocks - 1] |= (~block_type(0) << old_bits_in_last_block);
    }

    m_num_bits = num_bits;
    sanitize();
}

bool dynamic_bitset::empty() const { return m_num_bits == 0; }

bool dynamic_bitset::none() const
{
    if (empty()) return true;

    for (size_t i = 0; i < m_num_blocks - 1; ++i)
        if (m_blocks[i] != 0) return false;

    block_type mask       = ~block_type(0);
    size_t     extra_bits = unused_bits();
    if (extra_bits > 0) mask = ~(mask << (bits_per_block - extra_bits));

    return (m_blocks[m_num_blocks - 1] & mask) == 0;
}

bool dynamic_bitset::any() const { return !none(); }

bool dynamic_bitset::all() const
{
    if (empty()) return true;

    for (size_t i = 0; i < m_num_blocks - 1; ++i)
        if (m_blocks[i] != ~block_type(0)) return false;

    size_t extra_bits = unused_bits();
    if (extra_bits > 0)
    {
        block_type mask = (~block_type(0)) >> extra_bits;
        return (m_blocks[m_num_blocks - 1] & mask) == mask;
    }
    else
        return m_blocks[m_num_blocks - 1] == ~block_type(0);
}

size_t dynamic_bitset::count() const
{
    if (m_num_blocks == 0) return 0;

    size_t count = 0;

    constexpr size_t UNROLL_COUNT = 8;
    size_t           i            = 0;

    for (; i + UNROLL_COUNT <= m_num_blocks; i += UNROLL_COUNT)
        count += popcount(m_blocks[i]) + popcount(m_blocks[i + 1]) + popcount(m_blocks[i + 2]) +
                 popcount(m_blocks[i + 3]) + popcount(m_blocks[i + 4]) + popcount(m_blocks[i + 5]) +
                 popcount(m_blocks[i + 6]) + popcount(m_blocks[i + 7]);

    for (; i < m_num_blocks; ++i) count += popcount(m_blocks[i]);

    return count;
}

std::string dynamic_bitset::to_string(char zero, char one) const
{
    std::string result(m_num_bits, zero);

    for (size_t i = 0; i < m_num_bits; ++i)
    {
        size_t bit_pos   = m_num_bits - i - 1;
        size_t block_idx = i / bits_per_block;
        size_t bit_idx   = i % bits_per_block;

        if ((m_blocks[block_idx] & (block_type(1) << bit_idx)) != 0) result[bit_pos] = one;
    }

    return result;
}

dynamic_bitset& dynamic_bitset::operator&=(const dynamic_bitset& other)
{
    if (size() != other.size())
        throw std::invalid_argument("Bitsets must have the same size for bitwise AND operation");

    constexpr size_t UNROLL_COUNT = 4;
    size_t           i            = 0;

    for (; i + UNROLL_COUNT <= m_num_blocks; i += UNROLL_COUNT)
    {
        m_blocks[i] &= other.m_blocks[i];
        m_blocks[i + 1] &= other.m_blocks[i + 1];
        m_blocks[i + 2] &= other.m_blocks[i + 2];
        m_blocks[i + 3] &= other.m_blocks[i + 3];
    }

    for (; i < m_num_blocks; ++i) m_blocks[i] &= other.m_blocks[i];

    return *this;
}

dynamic_bitset& dynamic_bitset::operator|=(const dynamic_bitset& other)
{
    if (size() != other.size()) throw std::invalid_argument("Bitsets must have the same size for bitwise OR operation");

    constexpr size_t UNROLL_COUNT = 4;
    size_t           i            = 0;

    for (; i + UNROLL_COUNT <= m_num_blocks; i += UNROLL_COUNT)
    {
        m_blocks[i] |= other.m_blocks[i];
        m_blocks[i + 1] |= other.m_blocks[i + 1];
        m_blocks[i + 2] |= other.m_blocks[i + 2];
        m_blocks[i + 3] |= other.m_blocks[i + 3];
    }

    for (; i < m_num_blocks; ++i) m_blocks[i] |= other.m_blocks[i];

    return *this;
}

dynamic_bitset& dynamic_bitset::operator^=(const dynamic_bitset& other)
{
    if (size() != other.size())
        throw std::invalid_argument("Bitsets must have the same size for bitwise XOR operation");

    constexpr size_t UNROLL_COUNT = 4;
    size_t           i            = 0;

    for (; i + UNROLL_COUNT <= m_num_blocks; i += UNROLL_COUNT)
    {
        m_blocks[i] ^= other.m_blocks[i];
        m_blocks[i + 1] ^= other.m_blocks[i + 1];
        m_blocks[i + 2] ^= other.m_blocks[i + 2];
        m_blocks[i + 3] ^= other.m_blocks[i + 3];
    }

    for (; i < m_num_blocks; ++i) m_blocks[i] ^= other.m_blocks[i];

    return *this;
}

bool dynamic_bitset::operator==(const dynamic_bitset& other) const
{
    if (m_num_bits != other.m_num_bits) return false;
    if (m_num_blocks == 0) return true;

    return std::memcmp(m_blocks, other.m_blocks, m_num_blocks * sizeof(block_type)) == 0;
}

bool dynamic_bitset::operator!=(const dynamic_bitset& other) const { return !(*this == other); }

dynamic_bitset operator&(const dynamic_bitset& lhs, const dynamic_bitset& rhs)
{
    dynamic_bitset result(lhs);
    result &= rhs;
    return result;
}

dynamic_bitset operator|(const dynamic_bitset& lhs, const dynamic_bitset& rhs)
{
    dynamic_bitset result(lhs);
    result |= rhs;
    return result;
}

dynamic_bitset operator^(const dynamic_bitset& lhs, const dynamic_bitset& rhs)
{
    dynamic_bitset result(lhs);
    result ^= rhs;
    return result;
}

dynamic_bitset dynamic_bitset::operator~() const
{
    dynamic_bitset result(*this);
    result.flip();
    return result;
}

std::ostream& operator<<(std::ostream& os, const dynamic_bitset& bs) { return os << bs.to_string(); }
