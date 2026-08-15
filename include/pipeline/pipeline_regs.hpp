#pragma once
#include <cstdint>
#include "../memory/memory_types.hpp"
#include "../datapath/alu_types.hpp"

struct FetchSig {
    uint32_t        PCJ = 0x0;
};
struct DecodeSig {
    uint32_t        SA = 0x0;
    uint32_t        SB = 0x0;
};
struct ExecuteSig {
    uint32_t        MA = 0x0;
    uint32_t        MB = 0x0;
    ALUOp           FS = ALUOp::ADD;
};
struct MemorySig {
    bool            MW = false;
    uint32_t        MD = 0x0;
    MemSign         MSN = MemSign::S;
    MemSize         MSZ = MemSize::BYTE;
};
struct WriteBackSig {
    bool            LD = false;
    uint32_t        DR = 0x0;
};

struct IF_ID {
    uint32_t    pc_next = 0;
    uint32_t    pc_curr = 0;
    uint32_t    raw_instr = 0;
    bool        bubble = false;

    bool        stall = false;
};

struct ID_EX {
    uint32_t        pc_curr = 0;
    uint32_t        pc_next = 0;
    uint32_t        srcA = 0;
    uint32_t        srcB = 0;
    uint32_t        imm = 0;

    uint32_t        EX_SA;
    uint32_t        EX_SB;
    
    ExecuteSig      es;
    MemorySig       ms;
    WriteBackSig    wbs;

    bool            bubble = false;
    bool            halt = false;
    uint32_t        opcode;
};

struct EX_MEM {
    uint32_t        aluResult = 0;
    uint32_t        data = 0;
    uint32_t        pc_next = 0;

    MemorySig       ms;
    WriteBackSig    wbs;

    bool            halt = false;
};

struct MEM_WB {
    uint32_t    Dout = 0;

    uint32_t    DR = 0;
    bool        LD = false;
};