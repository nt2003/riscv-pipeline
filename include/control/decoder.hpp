#pragma once
#include <cstdint>

enum class InstrType {
    R_type, I_type, S_type, B_type, U_type, J_type
};

struct DecodedInstr {
    InstrType   type;
    uint32_t    opcode;
    uint32_t    rd;
    uint32_t    rs1;
    uint32_t    rs2;
    uint32_t    funct3;
    uint32_t    funct7;
    uint32_t    imm;
};

uint32_t extractOpcode(uint32_t instr);
uint32_t extractRd(uint32_t instr);
uint32_t extractRs1(uint32_t instr);
uint32_t extractRs2(uint32_t instr);
uint32_t extractFunct3(uint32_t instr);
uint32_t extractFunct7(uint32_t instr);

InstrType getType(uint32_t instr);

uint32_t extractImm(uint32_t instr, InstrType type);

DecodedInstr decodeInstr(uint32_t instr);







