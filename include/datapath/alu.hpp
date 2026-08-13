#pragma once
#include "../control/decoder.hpp"
#include "alu_types.hpp"

class ALU {
    private:
        bool Z_flag = false;

        bool N_flag = false;

        bool C_flag = false;

        bool V_flag = false;

        uint32_t inputA = 0x0;

        uint32_t inputB = 0x0;

        ALUOp op;

    public:
        ALU(ALUOp op = ALUOp::ADD);

        void setAluData(uint32_t inputA, uint32_t inputB);
        void setAluOp(ALUOp op);

        uint32_t output();
        CompareSig getCompareSig();
        ALUFlags getFlags();

        void resetFlags();
        bool getZ();
        bool getN();
        bool getC();
        bool getV();
};