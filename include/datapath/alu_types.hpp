#pragma once

struct CompareSig {
    bool equal;
    bool lessThan;
    bool lessThanU;
};

enum class ALUOp {
    ADD, SUB, XOR, AND, OR, SLL, SRL, SRA, SLT, SLTU
};