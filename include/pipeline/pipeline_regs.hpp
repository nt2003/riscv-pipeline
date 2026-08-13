#pragma once
#include <cstdint>

enum class MemSize {
    BYTE, HALFWORD, WORD
};

enum class MemSign {
    U, S
};

struct IF_ID {
    uint32_t    pc_next;
    uint32_t    pc_curr;
    uint32_t    raw_instr;
    bool        bubble;
};

struct ID_EX {
    uint32_t        pc_curr;
    uint32_t        srcA;
    uint32_t        srcB;
    uint32_t        imm;

    ExecuteSig      es;
    MemorySig       ms;
    WriteBackSig    wbs;

    bool            bubble;
};

struct EX_MEM {
    uint32_t        aluResult;
    uint32_t        data;

    MemorySig       ms;
    WriteBackSig    wbs;
};

struct MEM_WB {
    uint32_t    DR;
    bool        LD;
    uint32_t    Dout;
};