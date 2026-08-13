#include "../external/doctest.h"
#include "../include/datapath/reg_file.hpp"
#include <iostream>

TEST_CASE("RegFile - Constructor") {
    RegFile rf(32);

    std::vector<uint32_t> regs = rf.getRegFile();

    REQUIRE(regs.size() == 32);

    // All registers initially zero
    for (uint32_t i = 0; i < 32; i++) {
        CHECK(regs[i] == 0);
    }

    // x0 specifically must be zero
    CHECK(regs[0] == 0);
}


TEST_CASE("RegFile - Default Read") {
    RegFile rf(32);

    rf.setRegSigs({
        0,      // DR
        0,      // SA
        0,      // SB
        false   // LD
    });

    CHECK(rf.getDataA() == 0);
    CHECK(rf.getDataB() == 0);
}


TEST_CASE("RegFile - Write Register") {
    RegFile rf(32);

    // Write 123 to x1
    rf.setRegSigs({
        1,      // DR
        0,      // SA
        0,      // SB
        true    // LD
    });

    rf.writeReg(123);

    // Read x1
    rf.setRegSigs({
        0,
        1,      // SA
        1,      // SB
        false
    });

    CHECK(rf.getDataA() == 123);
    CHECK(rf.getDataB() == 123);
}


TEST_CASE("RegFile - Write Disabled") {
    RegFile rf(32);

    // Attempt to write x1 while LD is false
    rf.setRegSigs({
        1,      // DR
        0,
        0,
        false   // LD
    });

    rf.writeReg(123);

    // x1 should remain zero
    rf.setRegSigs({
        0,
        1,
        1,
        false
    });

    CHECK(rf.getDataA() == 0);
    CHECK(rf.getDataB() == 0);
}


TEST_CASE("RegFile - Register Zero Cannot Be Written") {
    RegFile rf(32);

    // Try writing to x0
    rf.setRegSigs({
        0,      // DR = x0
        0,
        0,
        true    // LD
    });

    rf.writeReg(0xFFFFFFFF);

    // x0 must remain zero
    rf.setRegSigs({
        0,
        0,
        0,
        false
    });

    CHECK(rf.getDataA() == 0);
    CHECK(rf.getDataB() == 0);
    CHECK(rf.getRegFile()[0] == 0);
}


TEST_CASE("RegFile - Register Zero Always Remains Zero") {
    RegFile rf(32);

    rf.setRegSigs({
        0,
        0,
        0,
        true
    });

    rf.writeReg(1);
    rf.writeReg(0x12345678);
    rf.writeReg(0xFFFFFFFF);
    rf.writeReg(0x80000000);

    rf.setRegSigs({
        0,
        0,
        0,
        false
    });

    CHECK(rf.getDataA() == 0);
    CHECK(rf.getDataB() == 0);
    CHECK(rf.getRegFile()[0] == 0);
}


TEST_CASE("RegFile - Read Port A") {
    RegFile rf(32);

    // x5 = 0x12345678
    rf.setRegSigs({
        5,
        0,
        0,
        true
    });

    rf.writeReg(0x12345678);

    // Read x5 through SA
    rf.setRegSigs({
        0,
        5,      // SA
        0,      // SB
        false
    });

    CHECK(rf.getDataA() == 0x12345678);
}


TEST_CASE("RegFile - Read Port B") {
    RegFile rf(32);

    // x10 = 0xABCDEF01
    rf.setRegSigs({
        10,
        0,
        0,
        true
    });

    rf.writeReg(0xABCDEF01);

    // Read x10 through SB
    rf.setRegSigs({
        0,
        0,
        10,     // SB
        false
    });

    CHECK(rf.getDataB() == 0xABCDEF01);
}


TEST_CASE("RegFile - Independent Read Ports") {
    RegFile rf(32);

    // x5 = 0x11111111
    rf.setRegSigs({
        5,
        0,
        0,
        true
    });

    rf.writeReg(0x11111111);

    // x10 = 0x22222222
    rf.setRegSigs({
        10,
        0,
        0,
        true
    });

    rf.writeReg(0x22222222);

    // Read x5 through A and x10 through B
    rf.setRegSigs({
        0,
        5,
        10,
        false
    });

    CHECK(rf.getDataA() == 0x11111111);
    CHECK(rf.getDataB() == 0x22222222);
}


TEST_CASE("RegFile - Same Register On Both Read Ports") {
    RegFile rf(32);

    // x7 = 0xDEADBEEF
    rf.setRegSigs({
        7,
        0,
        0,
        true
    });

    rf.writeReg(0xDEADBEEF);

    // Read x7 from both ports
    rf.setRegSigs({
        0,
        7,
        7,
        false
    });

    CHECK(rf.getDataA() == 0xDEADBEEF);
    CHECK(rf.getDataB() == 0xDEADBEEF);
}


TEST_CASE("RegFile - Overwrite Register") {
    RegFile rf(32);

    // x5 = 100
    rf.setRegSigs({
        5,
        0,
        0,
        true
    });

    rf.writeReg(100);

    // x5 = 200
    rf.writeReg(200);

    // Read x5
    rf.setRegSigs({
        0,
        5,
        5,
        false
    });

    CHECK(rf.getDataA() == 200);
    CHECK(rf.getDataB() == 200);
}


TEST_CASE("RegFile - Multiple Registers") {
    RegFile rf(32);

    // x1 = 10
    rf.setRegSigs({
        1,
        0,
        0,
        true
    });
    rf.writeReg(10);

    // x2 = 20
    rf.setRegSigs({
        2,
        0,
        0,
        true
    });
    rf.writeReg(20);

    // x3 = 30
    rf.setRegSigs({
        3,
        0,
        0,
        true
    });
    rf.writeReg(30);

    // Read x1 and x2
    rf.setRegSigs({
        0,
        1,
        2,
        false
    });

    CHECK(rf.getDataA() == 10);
    CHECK(rf.getDataB() == 20);

    // Read x3 and x1
    rf.setRegSigs({
        0,
        3,
        1,
        false
    });

    CHECK(rf.getDataA() == 30);
    CHECK(rf.getDataB() == 10);
}


TEST_CASE("RegFile - Boundary Registers") {
    RegFile rf(32);

    // x1
    rf.setRegSigs({
        1,
        0,
        0,
        true
    });

    rf.writeReg(0x11111111);

    // x31
    rf.setRegSigs({
        31,
        0,
        0,
        true
    });

    rf.writeReg(0xFFFFFFFF);

    // Read x1 and x31
    rf.setRegSigs({
        0,
        1,
        31,
        false
    });

    CHECK(rf.getDataA() == 0x11111111);
    CHECK(rf.getDataB() == 0xFFFFFFFF);
}


TEST_CASE("RegFile - All Registers") {
    RegFile rf(32);

    // Write unique value to every register except x0
    for (uint32_t i = 1; i < 32; i++) {
        rf.setRegSigs({
            i,      // DR
            0,      // SA
            0,      // SB
            true    // LD
        });

        rf.writeReg(i * 0x11111111);
    }

    // Verify every register
    for (uint32_t i = 1; i < 32; i++) {
        rf.setRegSigs({
            0,
            i,      // SA
            i,      // SB
            false
        });

        CHECK(rf.getDataA() == i * 0x11111111);
        CHECK(rf.getDataB() == i * 0x11111111);
    }

    // x0 must still be zero
    rf.setRegSigs({
        0,
        0,
        0,
        false
    });

    CHECK(rf.getDataA() == 0);
    CHECK(rf.getDataB() == 0);
}


TEST_CASE("RegFile - getRegFile") {
    RegFile rf(32);

    // x1
    rf.setRegSigs({
        1,
        0,
        0,
        true
    });
    rf.writeReg(0x12345678);

    // x2
    rf.setRegSigs({
        2,
        0,
        0,
        true
    });
    rf.writeReg(0xABCDEF01);

    std::vector<uint32_t> regs = rf.getRegFile();

    REQUIRE(regs.size() == 32);

    CHECK(regs[0] == 0);
    CHECK(regs[1] == 0x12345678);
    CHECK(regs[2] == 0xABCDEF01);

    // Unwritten registers remain zero
    CHECK(regs[3] == 0);
    CHECK(regs[31] == 0);
}


TEST_CASE("RegFile - Read Unaffected By Write Enable") {
    RegFile rf(32);

    // Write x5
    rf.setRegSigs({
        5,
        0,
        0,
        true
    });

    rf.writeReg(0xCAFEBABE);

    // Read x5 with LD disabled
    rf.setRegSigs({
        0,
        5,
        5,
        false
    });

    CHECK(rf.getDataA() == 0xCAFEBABE);
    CHECK(rf.getDataB() == 0xCAFEBABE);

    // Read x5 with LD enabled
    rf.setRegSigs({
        5,
        5,
        5,
        true
    });

    CHECK(rf.getDataA() == 0xCAFEBABE);
    CHECK(rf.getDataB() == 0xCAFEBABE);
}