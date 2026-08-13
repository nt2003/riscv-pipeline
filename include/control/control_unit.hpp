#pragma once
#include <cstdint>
#include "../pipeline/pipeline_regs.hpp"
#include "../datapath/alu_types.hpp"
#include "decoder.hpp"

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

class ControlUnit {
    private:
        FetchSig        fs;
        DecodeSig       ds;
        ExecuteSig      es;
        MemorySig       ms;
        WriteBackSig    wbs;
        CompareSig      cs;  

    public: 
        ControlUnit();

        void setSigs(DecodedInstr instr);
        void setCompareSig(CompareSig cs, uint32_t funct3);
    
        FetchSig getFetchSig();
        DecodeSig getDecodeSig();
        ExecuteSig getExecuteSig();
        MemorySig getMemorySig();
        WriteBackSig getWriteBackSig();

        ALUOp getALUSig();

    
};
