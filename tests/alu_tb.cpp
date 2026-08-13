#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../external/doctest.h"
#include "../include/datapath/alu.hpp"
#include <iostream>

TEST_CASE("ALU Constructor") {
    ALU alu;

    //Check starting flags
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    //Check default op
    alu.setAluData(1,1);
    CHECK(alu.output() == 2);
}
TEST_CASE("ADD") {
    ALU alu(ALUOp::ADD);

    // 0 + 1 = 1
    alu.setAluData(0, 1);
    CHECK(alu.output() == 1);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 1 + 1 = 2
    alu.setAluData(1, 1);
    CHECK(alu.output() == 2);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 0 + 0 = 0 -> Zero flag
    alu.setAluData(0, 0);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 0x7FFFFFFF + 1 = 0x80000000
    // Signed overflow: positive + positive = negative
    alu.setAluData(0x7FFFFFFF, 1);
    CHECK(alu.output() == 0x80000000);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == true);

    // 0xFFFFFFFF + 1 = 0
    // Unsigned carry, but no signed overflow
    alu.setAluData(0xFFFFFFFF, 1);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == true);
    CHECK(alu.getV() == false);

    // 0x80000000 + 0xFFFFFFFF = 0x7FFFFFFF
    // Negative + negative = positive -> signed overflow
    // Also produces an unsigned carry
    alu.setAluData(0x80000000, 0xFFFFFFFF);
    CHECK(alu.output() == 0x7FFFFFFF);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == true);
    CHECK(alu.getV() == true);

    // 0xFFFFFFFF + 0xFFFFFFFF = 0xFFFFFFFE
    // Tests negative result + carry
    alu.setAluData(0xFFFFFFFF, 0xFFFFFFFF);
    CHECK(alu.output() == 0xFFFFFFFE);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == true);
    CHECK(alu.getV() == false);
}
TEST_CASE("SUB") {
    ALU alu(ALUOp::SUB);

    // 5 - 3 = 2
    alu.setAluData(5, 3);
    CHECK(alu.output() == 2);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 3 - 3 = 0
    alu.setAluData(3, 3);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 3 - 5 = -2
    alu.setAluData(3, 5);
    CHECK(alu.output() == 0xFFFFFFFE);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == true);
    CHECK(alu.getV() == false);

    // INT_MIN - 1 = INT_MAX
    // Signed overflow
    alu.setAluData(0x80000000, 1);
    CHECK(alu.output() == 0x7FFFFFFF);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == true);
}


TEST_CASE("XOR") {
    ALU alu(ALUOp::XOR);

    // 0xFF00FF00 ^ 0x00FF00FF = 0xFFFFFFFF
    alu.setAluData(0xFF00FF00, 0x00FF00FF);
    CHECK(alu.output() == 0xFFFFFFFF);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // x ^ x = 0
    alu.setAluData(0x12345678, 0x12345678);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // Test alternating bits
    alu.setAluData(0xAAAAAAAA, 0x55555555);
    CHECK(alu.output() == 0xFFFFFFFF);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}


TEST_CASE("AND") {
    ALU alu(ALUOp::AND);

    // 0xFF00FF00 & 0x0F0F0F0F = 0x0F000F00
    alu.setAluData(0xFF00FF00, 0x0F0F0F0F);
    CHECK(alu.output() == 0x0F000F00);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // x & 0 = 0
    alu.setAluData(0xFFFFFFFF, 0);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // Test negative result
    alu.setAluData(0xFFFFFFFF, 0x80000000);
    CHECK(alu.output() == 0x80000000);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}


TEST_CASE("OR") {
    ALU alu(ALUOp::OR);

    // 0xFF000000 | 0x0000FFFF = 0xFF00FFFF
    alu.setAluData(0xFF000000, 0x0000FFFF);
    CHECK(alu.output() == 0xFF00FFFF);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 0 | 0 = 0
    alu.setAluData(0, 0);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // Test setting all bits
    alu.setAluData(0xAAAAAAAA, 0x55555555);
    CHECK(alu.output() == 0xFFFFFFFF);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}


TEST_CASE("SLL") {
    ALU alu(ALUOp::SLL);

    // 1 << 1 = 2
    alu.setAluData(1, 1);
    CHECK(alu.output() == 2);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 1 << 31 = 0x80000000
    alu.setAluData(1, 31);
    CHECK(alu.output() == 0x80000000);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // Shift to zero
    alu.setAluData(0x80000000, 1);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // RISC-V masks shift amount to 5 bits
    // 1 << 33 == 1 << 1
    alu.setAluData(1, 33);
    CHECK(alu.output() == 2);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}


TEST_CASE("SRL") {
    ALU alu(ALUOp::SRL);

    // 8 >> 1 = 4
    alu.setAluData(8, 1);
    CHECK(alu.output() == 4);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 0x80000000 >> 1 = 0x40000000
    alu.setAluData(0x80000000, 1);
    CHECK(alu.output() == 0x40000000);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 1 >> 1 = 0
    alu.setAluData(1, 1);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // RISC-V masks shift amount
    alu.setAluData(8, 33);
    CHECK(alu.output() == 4);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}


TEST_CASE("SRA") {
    ALU alu(ALUOp::SRA);

    // 8 >> 1 = 4
    alu.setAluData(8, 1);
    CHECK(alu.output() == 4);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // Arithmetic shift of negative number
    // 0x80000000 >> 1 = 0xC0000000
    alu.setAluData(0x80000000, 1);
    CHECK(alu.output() == 0xC0000000);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // -1 >> 1 = -1
    alu.setAluData(0xFFFFFFFF, 1);
    CHECK(alu.output() == 0xFFFFFFFF);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == true);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 1 >> 1 = 0
    alu.setAluData(1, 1);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}


TEST_CASE("SLT") {
    ALU alu(ALUOp::SLT);

    // 1 < 2 -> true
    alu.setAluData(1, 2);
    CHECK(alu.output() == 1);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 2 < 1 -> false
    alu.setAluData(2, 1);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // -1 < 1 -> true
    alu.setAluData(0xFFFFFFFF, 1);
    CHECK(alu.output() == 1);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // INT_MIN < 0 -> true
    alu.setAluData(0x80000000, 0);
    CHECK(alu.output() == 1);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}


TEST_CASE("SLTU") {
    ALU alu(ALUOp::SLTU);

    // 1 < 2 -> true
    alu.setAluData(1, 2);
    CHECK(alu.output() == 1);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 2 < 1 -> false
    alu.setAluData(2, 1);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 0xFFFFFFFF > 1 unsigned
    // Even though 0xFFFFFFFF is -1 signed
    alu.setAluData(0xFFFFFFFF, 1);
    CHECK(alu.output() == 0);
    CHECK(alu.getZ() == true);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);

    // 0 < 0xFFFFFFFF unsigned
    alu.setAluData(0, 0xFFFFFFFF);
    CHECK(alu.output() == 1);
    CHECK(alu.getZ() == false);
    CHECK(alu.getN() == false);
    CHECK(alu.getC() == false);
    CHECK(alu.getV() == false);
}