#pragma once
#include <cstdint>
#include "../memory/memory_types.hpp"

struct IF_ID {
    uint32_t    pc_next;
    uint32_t    pc_curr;
    uint32_t    raw_instr;
    bool        bubble;
};

struct ID_EX {
    uint32_t        pc_curr;
    uint32_t        pc_next;
    uint32_t        srcA;
    uint32_t        srcB;
    uint32_t        imm;

    ExecuteSig      es;
    MemorySig       ms;
    WriteBackSig    wbs;

    bool            bubble;
    bool            halt;
};

struct EX_MEM {
    uint32_t        aluResult;
    uint32_t        data;
    uint32_t        pc_next;

    MemorySig       ms;
    WriteBackSig    wbs;

    bool            bubble;
    bool            halt;
};

struct MEM_WB {
    uint32_t    Dout;

    uint32_t    DR;
    bool        LD;
    
    bool        bubble;
    bool        halt;
};