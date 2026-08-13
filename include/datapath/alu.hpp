#pragma once
#include "decoder.hpp"
#include "alu_types.hpp"

class ALU {
    private:
        bool Z_flag;

        bool N_flag;

        bool C_flag;

        bool V_flag;

        uint32_t inputA;

        uint32_t inputB;

        ALUOp op;

    public:
        ALU();

        void setAluData(uint32_t inputA, uint32_t inputB);
        void setAluOp(ALUOp op);

        uint32_t output();
        CompareSig getCompareSig();
        void resetFlags();
        bool getZ();
        bool getN();
        bool getC();
        bool getV();
};