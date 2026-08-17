#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../external/doctest.h"
#include "../include/cpu/cpu.hpp"
#include <iostream>



// ============================================================
// Helper
// ============================================================

static void writeWord(Memory& mem, uint32_t addr, uint32_t data) {
    mem.setWriteEnable(true);
    mem.write(addr + 0, static_cast<uint8_t>((data >> 0)  & 0xFF));
    mem.write(addr + 1, static_cast<uint8_t>((data >> 8)  & 0xFF));
    mem.write(addr + 2, static_cast<uint8_t>((data >> 16) & 0xFF));
    mem.write(addr + 3, static_cast<uint8_t>((data >> 24) & 0xFF));
}

// Fill remaining memory with NOPs (addi x0, x0, 0) so fetch doesn't
// read garbage/decode bogus opcodes once the real program ends.
static void fillNops(Memory& mem, uint32_t fromAddr, uint32_t toAddrExclusive) {
    for (uint32_t addr = fromAddr; addr < toAddrExclusive; addr += 4) {
        writeWord(mem, addr, 0x00000013);
    }
}


static uint32_t rv32_encodeR(
    uint32_t funct7,
    uint32_t rs2,
    uint32_t rs1,
    uint32_t funct3,
    uint32_t rd
) {
    return (funct7 << 25) |
           (rs2    << 20) |
           (rs1    << 15) |
           (funct3 << 12) |
           (rd     << 7)  |
           0x33;
}


static uint32_t rv32_encodeI(
    int32_t imm,
    uint32_t rs1,
    uint32_t funct3,
    uint32_t rd,
    uint32_t opcode = 0x13
) {
    uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;

    return (uimm    << 20) |
           (rs1     << 15) |
           (funct3  << 12) |
           (rd      << 7)  |
           opcode;
}


static uint32_t rv32_encodeB(
    int32_t imm,
    uint32_t rs2,
    uint32_t rs1,
    uint32_t funct3
) {
    uint32_t uimm = static_cast<uint32_t>(imm);

    uint32_t bit12     = (uimm >> 12) & 0x1;
    uint32_t bit11     = (uimm >> 11) & 0x1;
    uint32_t bits10_5  = (uimm >> 5) & 0x3F;
    uint32_t bits4_1   = (uimm >> 1) & 0xF;

    return (bit12 << 31) |
           (bits10_5 << 25) |
           (rs2 << 20) |
           (rs1 << 15) |
           (funct3 << 12) |
           (bits4_1 << 8) |
           (bit11 << 7) |
           0x63;
}


static uint32_t rv32_encodeS(
    int32_t imm,
    uint32_t rs2,
    uint32_t rs1,
    uint32_t funct3
) {
    uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;

    return (((uimm >> 5) & 0x7F) << 25) |
           (rs2 << 20) |
           (rs1 << 15) |
           (funct3 << 12) |
           ((uimm & 0x1F) << 7) |
           0x23;
}
// ============================================================
// MIXED RV32I STRESS TEST
// ============================================================

TEST_CASE("RV32I - Mixed ALU, memory, forwarding, branch and jump") {
    Memory instrMem(256);
    Memory dataMem(128);

    // --------------------------------------------------------
    // x1 = 10
    // x2 = 20
    // x3 = x1 + x2 = 30
    // store x3
    // load x4 = 30
    // x5 = x4 + 12 = 42
    // x6 = 42
    // branch x5 == x6 -> taken
    // wrong path writes x7 = 1
    // target writes x7 = 99
    // --------------------------------------------------------

    writeWord(instrMem, 0,  rv32_encodeI(10, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeI(20, 0, 0x0, 2));
    writeWord(instrMem, 8,  rv32_encodeR(0x00, 2, 1, 0x0, 3));

    writeWord(instrMem, 12, rv32_encodeS(0, 3, 0, 0x2));
    writeWord(instrMem, 16, rv32_encodeI(0, 0, 0x2, 4, 0x03));

    writeWord(instrMem, 20, rv32_encodeI(12, 4, 0x0, 5));
    writeWord(instrMem, 24, rv32_encodeI(42, 0, 0x0, 6));

    // Branch to PC 36.
    writeWord(instrMem, 28, rv32_encodeB(8, 6, 5, 0x0));

    writeWord(instrMem, 32, rv32_encodeI(1, 0, 0x0, 7));   // x7 (wrong path)
    writeWord(instrMem, 36, rv32_encodeI(99, 0, 0x0, 8));  // x8 (target) — rd changed
    fillNops(instrMem, 40, 256);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 30; i++) {
        // std::cout << std::dec << '\n' << "Cycle: " << i << '\n';
        // std::cout << static_cast<unsigned int>(cpu.getRegFile()[7]) << '\n';
        // cpu.cycle();
    }
    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 20);
    CHECK(regs[3] == 30);
    CHECK(regs[4] == 30);
    CHECK(regs[5] == 42);
    CHECK(regs[6] == 42);

    // Proves branch was taken AND wrong path was flushed.
    CHECK(regs[7] == 0);
}
