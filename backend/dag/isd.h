#ifndef __BACKEND_DAG_ISD_H__
#define __BACKEND_DAG_ISD_H__

#include <debug.h>

namespace BE
{
    namespace DAG
    {
        enum class ISD : unsigned
        {
            // Terminators
            RET = 1,
            BR,
            BRCOND,

            // Token for memory/side-effect ordering
            ENTRY_TOKEN,
            TOKEN_FACTOR,

            // Values and constants
            COPY,
            REG,
            PHI,
            CONST_I32,
            CONST_I64,
            CONST_F32,
            SYMBOL,  // global/function symbol
            LABEL,   // basic block label id

            // Memory
            LOAD,
            STORE,
            FRAME_INDEX,  // Abstract stack slot index
            GEP,

            // Integer ops
            ADD,
            SUB,
            MUL,
            DIV,
            MOD,
            SHL,
            ASHR,
            LSHR,
            AND,
            OR,
            XOR,

            // Floating ops
            FADD,
            FSUB,
            FMUL,
            FDIV,

            // Casts/extends
            ZEXT,
            SITOFP,
            FPTOSI,

            // Compares
            ICMP,  // cond code in node imm
            FCMP,  // cond code in node imm

            // Calls
            CALL,
        };

        static inline const char* toString(ISD op)
        {
            switch (op)
            {
                case ISD::RET: return "RET";
                case ISD::BR: return "BR";
                case ISD::BRCOND: return "BRCOND";
                case ISD::ENTRY_TOKEN: return "ENTRY_TOKEN";
                case ISD::TOKEN_FACTOR: return "TOKEN_FACTOR";
                case ISD::COPY: return "COPY";
                case ISD::REG: return "REG";
                case ISD::PHI: return "PHI";
                case ISD::CONST_I32: return "CONST_I32";
                case ISD::CONST_I64: return "CONST_I64";
                case ISD::CONST_F32: return "CONST_F32";
                case ISD::SYMBOL: return "SYMBOL";
                case ISD::LABEL: return "LABEL";
                case ISD::LOAD: return "LOAD";
                case ISD::STORE: return "STORE";
                case ISD::FRAME_INDEX: return "FRAME_INDEX";
                case ISD::GEP: return "GEP";
                case ISD::ADD: return "ADD";
                case ISD::SUB: return "SUB";
                case ISD::MUL: return "MUL";
                case ISD::DIV: return "DIV";
                case ISD::MOD: return "MOD";
                case ISD::SHL: return "SHL";
                case ISD::ASHR: return "ASHR";
                case ISD::LSHR: return "LSHR";
                case ISD::AND: return "AND";
                case ISD::OR: return "OR";
                case ISD::XOR: return "XOR";
                case ISD::FADD: return "FADD";
                case ISD::FSUB: return "FSUB";
                case ISD::FMUL: return "FMUL";
                case ISD::FDIV: return "FDIV";
                case ISD::ZEXT: return "ZEXT";
                case ISD::SITOFP: return "SITOFP";
                case ISD::FPTOSI: return "FPTOSI";
                case ISD::ICMP: return "ICMP";
                case ISD::FCMP: return "FCMP";
                case ISD::CALL: return "CALL";
                default: ERROR("Unknown ISD opcode"); return "UNKNOWN";
            }
        }
    }  // namespace DAG
}  // namespace BE

#endif  // __BACKEND_DAG_ISD_H__
