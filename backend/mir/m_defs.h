#ifndef __BACKEND_MIR_DEFS_H__
#define __BACKEND_MIR_DEFS_H__

#include <cstdint>
#include <string>
#include <debug.h>

namespace ME
{
    enum class DataType;
}

namespace BE
{
    struct DataType
    {
        enum class Type
        {
            INT,
            FLOAT,
            TOKEN  // Zero-width type for dependency chains
        };
        enum class Length
        {
            B32,
            B64
        };
        Type   dt;
        Length dl;
        DataType(const DataType& other)
        {
            this->dt = other.dt;
            this->dl = other.dl;
        }
        DataType operator=(const DataType& other)
        {
            this->dt = other.dt;
            this->dl = other.dl;
            return *this;
        }
        DataType(Type dt, Length dl) : dt(dt), dl(dl) {}
        bool operator==(const DataType& other) const { return this->dt == other.dt && this->dl == other.dl; }
        bool equal(const DataType& other) const { return this->dt == other.dt && this->dl == other.dl; }
        bool equal(const DataType* other) const
        {
            if (!other) return false;
            return this->dt == other->dt && this->dl == other->dl;
        }
        int getDataWidth()
        {
            switch (dl)
            {
                case Length::B32: return 4;
                case Length::B64: return 8;
            }
            return 0;
        }
        std::string toString()
        {
            std::string ret;
            if (dt == Type::INT) ret += 'i';
            if (dt == Type::FLOAT) ret += 'f';
            if (dl == Length::B32) ret += "32";
            if (dl == Length::B64) ret += "64";
            return ret;
        }
    };

    extern DataType *I32, *I64, *F32, *F64, *PTR, *TOKEN;

    enum class InstKind
    {
        NOP    = 0,   // 空，可作为 comment 使用
        PHI    = 1,   // 不同路径选择值
        MOVE   = 2,   // 数据拷贝
        SELECT = 3,   // 条件选择
        LSLOT  = 4,   // 内存槽加载
        SSLOT  = 5,   // 内存槽存储
        TARGET = 100  // 目标相关指令
    };

    class Register
    {
      public:
        uint32_t  rId;
        DataType* dt;

        bool isVreg;

      public:
        Register(int reg = 0, DataType* dataType = nullptr, bool isV = false) : rId(reg), dt(dataType), isVreg(isV) {}

      public:
        bool operator<(Register other) const;
        bool operator==(Register other) const;
    };

    class Operand
    {
      public:
        enum class Type
        {
            REG         = 0,
            IMMI32      = 1,
            IMMI64      = 2,
            IMMF32      = 3,
            IMMF64      = 4,
            FRAME_INDEX = 5  // Abstract stack slot reference
        };

      public:
        DataType* dt;
        Type      ot;

      public:
        Operand(DataType* dt, Type ot) : dt(dt), ot(ot) {}
        virtual ~Operand() = default;
    };

    class RegOperand : public Operand
    {
      public:
        Register reg;

      public:
        RegOperand(Register reg) : Operand(reg.dt, Operand::Type::REG), reg(reg) {}
    };

    class I32Operand : public Operand
    {
      public:
        int val;

      public:
        I32Operand(int value) : Operand(I32, Operand::Type::IMMI32), val(value) {}
    };

    class F32Operand : public Operand
    {
      public:
        float val;

      public:
        F32Operand(float value) : Operand(F32, Operand::Type::IMMF32), val(value) {}
    };

    class FrameIndexOperand : public Operand
    {
      public:
        int frameIndex;

      public:
        FrameIndexOperand(int fi) : Operand(I64, Operand::Type::FRAME_INDEX), frameIndex(fi) {}
    };

    Register getVReg(DataType* dt);
}  // namespace BE

#endif  // __BACKEND_MIR_DEFS_H__
