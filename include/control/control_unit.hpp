#pragma once
#include <cstdint>
#include "../pipeline/pipeline_regs.hpp"
#include "../datapath/alu_types.hpp"
#include "decoder.hpp"

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

        // ALUOp getALUSig();

    
};
