#pragma once

struct CompareSig {
    bool equal = false;
    bool lessThan = false;
    bool lessThanU = false;
};

struct ALUFlags {
    bool Z_flag; 
    bool N_flag;
    bool C_flag;
    bool V_flag;
};

enum class ALUOp {
    ADD, SUB, XOR, AND, OR, SLL, SRL, SRA, SLT, SLTU
};