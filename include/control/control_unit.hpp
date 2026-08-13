#pragma once
#include <cstdint>
#include "../pipeline/pipeline_regs.hpp"
#include "../datapath/alu_types.hpp"

struct FetchSig {
    uint32_t        PCJ;
};
struct DecodeSig {
    uint32_t        SA;
    uint32_t        SB;
};
struct ExecuteSig {
    uint32_t        MA;
    uint32_t        MB;
    ALUOp           FS;
};
struct MemorySig {
    bool            MW;
    uint32_t        MD;
    MemSign         MSN;
    MemSize         MSZ;
};
struct WriteBackSig {
    bool            LD;
    uint32_t        DR;
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
        void setCompareSig(CompareSig cs);
    
        FetchSig getFetchSig();
        DecodeSig getDecodeSig();
        ExecuteSig getExecuteSig();
        MemorySig getMemorySig();
        WriteBackSig getWriteBackSig();

        ALUOp getALUSig();

    
};
