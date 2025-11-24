#ifndef __UTILS_DYNAMIC_BITSET_H__
#define __UTILS_DYNAMIC_BITSET_H__

#include <climits>
#include <string>
#include <cstddef>
#include <iostream>
#include <stdexcept>

class dynamic_bitset
{
  public:
    using block_type                       = unsigned long;
    static constexpr size_t bits_per_block = sizeof(block_type) * CHAR_BIT;

  private:
    block_type* m_blocks;
    size_t      m_num_bits;
    size_t      m_num_blocks;

  public:
    dynamic_bitset();
    explicit dynamic_bitset(size_t num_bits, unsigned long value = 0);
    dynamic_bitset(
        const std::string& str, size_t pos = 0, size_t n = std::string::npos, char zero = '0', char one = '1');
    dynamic_bitset(const dynamic_bitset& other);
    dynamic_bitset(dynamic_bitset&& other) noexcept;
    ~dynamic_bitset();

    dynamic_bitset& operator=(const dynamic_bitset& other);
    dynamic_bitset& operator=(dynamic_bitset&& other) noexcept;

    dynamic_bitset& set(size_t pos, bool value = true);
    dynamic_bitset& reset(size_t pos);
    dynamic_bitset& flip(size_t pos);
    dynamic_bitset& set_range(size_t start, size_t count, bool value = true);

    dynamic_bitset& set();
    dynamic_bitset& reset();
    dynamic_bitset& flip();

    bool test(size_t pos) const;
    bool operator[](size_t pos) const;

    size_t size() const;
    void   resize(size_t num_bits, bool value = false);

    bool empty() const;
    bool none() const;
    bool any() const;
    bool all() const;

    size_t      count() const;
    std::string to_string(char zero = '0', char one = '1') const;

    dynamic_bitset& operator&=(const dynamic_bitset& other);
    dynamic_bitset& operator|=(const dynamic_bitset& other);
    dynamic_bitset& operator^=(const dynamic_bitset& other);
    dynamic_bitset  operator~() const;

    bool operator==(const dynamic_bitset& other) const;
    bool operator!=(const dynamic_bitset& other) const;

  private:
    static inline size_t blocks_required(size_t num_bits) { return (num_bits + bits_per_block - 1) / bits_per_block; }

    inline size_t unused_bits() const
    {
        return m_num_bits ? (bits_per_block - (m_num_bits % bits_per_block)) % bits_per_block : 0;
    }

    inline void check_bounds(size_t pos) const
    {
        if (pos >= m_num_bits) throw std::out_of_range("Position out of range");
    }

    void          sanitize();
    static size_t popcount(block_type block);
};

dynamic_bitset operator&(const dynamic_bitset& lhs, const dynamic_bitset& rhs);
dynamic_bitset operator|(const dynamic_bitset& lhs, const dynamic_bitset& rhs);
dynamic_bitset operator^(const dynamic_bitset& lhs, const dynamic_bitset& rhs);
std::ostream&  operator<<(std::ostream& os, const dynamic_bitset& bs);

#endif  // __UTILS_DYNAMIC_BITSET_H__
