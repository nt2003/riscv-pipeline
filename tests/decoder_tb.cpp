#include "../external/doctest.h"
#include "../include/control/decoder.hpp"
#include <iostream>


// ============================================================
// Field Extraction
// ============================================================

TEST_CASE("Decoder - extractOpcode") {

    // add x5, x6, x7
    uint32_t instr = 0x007302B3;

    CHECK(extractOpcode(instr) == 0x33);
}


TEST_CASE("Decoder - extractRd") {

    // rd = 5
    uint32_t instr = 0x007302B3;

    CHECK(extractRd(instr) == 5);
}


TEST_CASE("Decoder - extractRs1") {

    // rs1 = 6
    uint32_t instr = 0x007302B3;

    CHECK(extractRs1(instr) == 6);
}


TEST_CASE("Decoder - extractRs2") {

    // rs2 = 7
    uint32_t instr = 0x007302B3;

    CHECK(extractRs2(instr) == 7);
}


TEST_CASE("Decoder - extractFunct3") {

    // ADD funct3 = 0
    uint32_t instr = 0x007302B3;

    CHECK(extractFunct3(instr) == 0);
}


TEST_CASE("Decoder - extractFunct7") {

    // ADD funct7 = 0
    uint32_t instr = 0x007302B3;

    CHECK(extractFunct7(instr) == 0);
}


// ============================================================
// Field Extraction - Nonzero Values
// ============================================================

TEST_CASE("Decoder - Extract All R-Type Fields") {

    // sub x10, x11, x12
    //
    // rd    = 10
    // rs1   = 11
    // rs2   = 12
    // funct3 = 0
    // funct7 = 0x20
    // opcode = 0x33

    uint32_t instr = 0x40C58533;

    CHECK(extractOpcode(instr) == 0x33);
    CHECK(extractRd(instr) == 10);
    CHECK(extractRs1(instr) == 11);
    CHECK(extractRs2(instr) == 12);
    CHECK(extractFunct3(instr) == 0);
    CHECK(extractFunct7(instr) == 0x20);
}


// ============================================================
// Instruction Type
// ============================================================

TEST_CASE("Decoder - getType R-Type") {

    // ADD
    uint32_t instr = 0x007302B3;

    CHECK(getType(instr) == InstrType::R_type);
}


TEST_CASE("Decoder - getType I-Type") {

    // ADDI
    uint32_t instr = 0x06430293;

    CHECK(getType(instr) == InstrType::I_type);
}


TEST_CASE("Decoder - getType S-Type") {

    // SW
    uint32_t instr = 0x00732223;

    CHECK(getType(instr) == InstrType::S_type);
}


TEST_CASE("Decoder - getType B-Type") {

    // BEQ
    uint32_t instr = 0x00628863;

    CHECK(getType(instr) == InstrType::B_type);
}


TEST_CASE("Decoder - getType U-Type") {

    // LUI
    uint32_t instr = 0x123452B7;

    CHECK(getType(instr) == InstrType::U_type);
}


TEST_CASE("Decoder - getType J-Type") {

    // JAL
    uint32_t instr = 0x010002EF;

    CHECK(getType(instr) == InstrType::J_type);
}


// ============================================================
// I-Type Immediate
// ============================================================

TEST_CASE("Decoder - I-Type Immediate Positive") {

    // addi x5, x6, 100

    uint32_t instr = 0x06430293;

    CHECK(extractImm(instr, InstrType::I_type) == 100);
}


TEST_CASE("Decoder - I-Type Immediate Negative") {

    // addi x5, x6, -100
    //
    // 12-bit immediate = 0xF9C

    uint32_t instr = 0xF9C30293;

    CHECK(extractImm(instr, InstrType::I_type)
          == static_cast<uint32_t>(-100));
}


TEST_CASE("Decoder - I-Type Immediate Maximum Positive") {

    // 12-bit signed maximum = 2047

    uint32_t instr = 0x7FF30293;

    CHECK(extractImm(instr, InstrType::I_type) == 2047);
}


TEST_CASE("Decoder - I-Type Immediate Minimum Negative") {

    // 12-bit signed minimum = -2048

    uint32_t instr = 0x80030293;

    CHECK(extractImm(instr, InstrType::I_type)
          == static_cast<uint32_t>(-2048));
}


// ============================================================
// S-Type Immediate
// ============================================================

TEST_CASE("Decoder - S-Type Immediate Positive") {

    // sw x7, 100(x6)

    uint32_t instr = 0x06732223;

    CHECK(extractImm(instr, InstrType::S_type) == 100);
}


TEST_CASE("Decoder - S-Type Immediate Negative") {

    // sw x7, -100(x6)

    uint32_t instr = 0xF87322A3;

    CHECK(extractImm(instr, InstrType::S_type)
          == static_cast<uint32_t>(-123));
}


TEST_CASE("Decoder - S-Type Immediate Maximum Positive") {

    // 12-bit signed max = 2047

    uint32_t instr = 0x7E732FA3;

    CHECK(extractImm(instr, InstrType::S_type) == 2047);
}


TEST_CASE("Decoder - S-Type Immediate Minimum Negative") {

    // 12-bit signed min = -2048

    uint32_t instr = 0x80732023;

    CHECK(extractImm(instr, InstrType::S_type)
          == static_cast<uint32_t>(-2048));
}


// ============================================================
// B-Type Immediate
// ============================================================

TEST_CASE("Decoder - B-Type Immediate Zero") {

    // BEQ x5, x6, 0

    uint32_t instr = 0x00628063;

    CHECK(extractImm(instr, InstrType::B_type) == 0);
}


TEST_CASE("Decoder - B-Type Immediate Positive") {

    // BEQ x5, x6, 8

    uint32_t instr = 0x00628463;

    CHECK(extractImm(instr, InstrType::B_type) == 8);
}


TEST_CASE("Decoder - B-Type Immediate Negative") {

    // BEQ x5, x6, -8

    uint32_t instr = 0xFE628CE3;

    CHECK(extractImm(instr, InstrType::B_type)
          == static_cast<uint32_t>(-8));
}


TEST_CASE("Decoder - B-Type Immediate Maximum Positive") {

    // Maximum positive branch offset = 4094

    uint32_t instr = 0x7E628FE3;

    CHECK(extractImm(instr, InstrType::B_type) == 4094);
}


TEST_CASE("Decoder - B-Type Immediate Minimum Negative") {

    // Minimum branch offset = -4096

    uint32_t instr = 0x80628063;

    CHECK(extractImm(instr, InstrType::B_type)
          == static_cast<uint32_t>(-4096));
}


// ============================================================
// U-Type Immediate
// ============================================================

TEST_CASE("Decoder - U-Type Immediate") {

    // lui x5, 0x12345

    uint32_t instr = 0x123452B7;

    CHECK(extractImm(instr, InstrType::U_type)
          == 0x12345000);
}


TEST_CASE("Decoder - U-Type Immediate Zero") {

    uint32_t instr = 0x000002B7;

    CHECK(extractImm(instr, InstrType::U_type) == 0);
}


TEST_CASE("Decoder - U-Type Immediate High Bit") {

    uint32_t instr = 0x800002B7;

    CHECK(extractImm(instr, InstrType::U_type)
          == 0x80000000);
}


// ============================================================
// J-Type Immediate
// ============================================================

TEST_CASE("Decoder - J-Type Immediate Zero") {

    // jal x5, 0

    uint32_t instr = 0x000002EF;

    CHECK(extractImm(instr, InstrType::J_type) == 0);
}


TEST_CASE("Decoder - J-Type Immediate Positive") {

    // jal x5, 16

    uint32_t instr = 0x010002EF;

    CHECK(extractImm(instr, InstrType::J_type) == 16);
}


TEST_CASE("Decoder - J-Type Immediate Negative") {

    // jal x5, -16

    uint32_t instr = 0xFF1FF2EF;

    CHECK(extractImm(instr, InstrType::J_type)
          == static_cast<uint32_t>(-16));
}


// ============================================================
// decodeInstr() Integration Tests
// ============================================================

TEST_CASE("Decoder - decodeInstr R-Type") {

    // add x5, x6, x7

    uint32_t instr = 0x007302B3;

    DecodedInstr d = decodeInstr(instr);

    CHECK(d.type == InstrType::R_type);

    CHECK(d.opcode == 0x33);
    CHECK(d.rd == 5);
    CHECK(d.rs1 == 6);
    CHECK(d.rs2 == 7);

    CHECK(d.funct3 == 0);
    CHECK(d.funct7 == 0);

    CHECK(d.imm == 0);
}


TEST_CASE("Decoder - decodeInstr I-Type") {

    // addi x5, x6, 100

    uint32_t instr = 0x06430293;

    DecodedInstr d = decodeInstr(instr);

    CHECK(d.type == InstrType::I_type);

    CHECK(d.opcode == 0x13);
    CHECK(d.rd == 5);
    CHECK(d.rs1 == 6);

    CHECK(d.funct3 == 0);

    CHECK(d.imm == 100);
}


TEST_CASE("Decoder - decodeInstr S-Type") {

    // sw x7, 100(x6)

    uint32_t instr = 0x06732223;

    DecodedInstr d = decodeInstr(instr);

    CHECK(d.type == InstrType::S_type);

    CHECK(d.opcode == 0x23);
    CHECK(d.rs1 == 6);
    CHECK(d.rs2 == 7);

    CHECK(d.funct3 == 2);

    CHECK(d.imm == 100);
}


TEST_CASE("Decoder - decodeInstr B-Type") {

    // beq x5, x6, 8

    uint32_t instr = 0x00628463;

    DecodedInstr d = decodeInstr(instr);

    CHECK(d.type == InstrType::B_type);

    CHECK(d.opcode == 0x63);
    CHECK(d.rs1 == 5);
    CHECK(d.rs2 == 6);

    CHECK(d.funct3 == 0);

    CHECK(d.imm == 8);
}


TEST_CASE("Decoder - decodeInstr U-Type") {

    // lui x5, 0x12345

    uint32_t instr = 0x123452B7;

    DecodedInstr d = decodeInstr(instr);

    CHECK(d.type == InstrType::U_type);

    CHECK(d.opcode == 0x37);
    CHECK(d.rd == 5);

    CHECK(d.imm == 0x12345000);
}


TEST_CASE("Decoder - decodeInstr J-Type") {

    // jal x5, 16

    uint32_t instr = 0x010002EF;

    DecodedInstr d = decodeInstr(instr);

    CHECK(d.type == InstrType::J_type);

    CHECK(d.opcode == 0x6F);
    CHECK(d.rd == 5);

    CHECK(d.imm == 16);
}