#include "../external/doctest.h"
#include "../include/cpu/cpu.hpp"
#include <iostream>
#include <cstdint>
#include <vector>


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

// ============================================================
// Constructor
// ============================================================

TEST_CASE("CPU - Constructor") {
    Memory instrMem(64);
    Memory dataMem(64);

    CPU cpu(instrMem, dataMem, 0);

    CHECK(cpu.isHalted() == false);

    std::vector<uint32_t> regs = cpu.getRegFile();
    CHECK(regs.size() == 32);
    for (uint32_t i = 0; i < 32; i++) {
        CHECK(regs[i] == 0);
    }
}

// ============================================================
// Fetch (isolated)
// ============================================================

TEST_CASE("CPU - Fetch First Instruction") {
    Memory instrMem(64);
    Memory dataMem(64);
    writeWord(instrMem, 0, 0x00500093); // addi x1, x0, 5

    CPU cpu(instrMem, dataMem, 0);
    IF_ID result = cpu.fetch();

    CHECK(result.pc_curr == 0);
    CHECK(result.pc_next == 4);
    CHECK(result.raw_instr == 0x00500093);
    CHECK(result.bubble == false);
}

TEST_CASE("CPU - Fetch Increments PC Across Calls") {
    Memory instrMem(64);
    Memory dataMem(64);
    writeWord(instrMem, 0, 0x00500093); // addi x1, x0, 5
    writeWord(instrMem, 4, 0x00A00113); // addi x2, x0, 10
    writeWord(instrMem, 8, 0x01400193); // addi x3, x0, 20

    CPU cpu(instrMem, dataMem, 0);

    IF_ID first = cpu.fetch();
    CHECK(first.pc_curr == 0);
    CHECK(first.pc_next == 4);

    IF_ID second = cpu.fetch();
    CHECK(second.pc_curr == 4);
    CHECK(second.pc_next == 8);

    IF_ID third = cpu.fetch();
    CHECK(third.pc_curr == 8);
    CHECK(third.pc_next == 12);
}

// ============================================================
// Full pipeline — single instruction writes back correctly
// ============================================================

TEST_CASE("CPU - Pipeline: ADDI writes back after 5 cycles") {
    Memory instrMem(64);
    Memory dataMem(64);

    writeWord(instrMem, 0, 0x00500093); // addi x1, x0, 5
    fillNops(instrMem, 4, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 5; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();
    CHECK(regs[1] == 5);
}

TEST_CASE("CPU - Pipeline: Multiple ADDIs write back in order") {
    Memory instrMem(64);
    Memory dataMem(64);

    writeWord(instrMem, 0, 0x00500093);  // addi x1, x0, 5
    writeWord(instrMem, 4, 0x00A00113);  // addi x2, x0, 10
    writeWord(instrMem, 8, 0x01400193);  // addi x3, x0, 20
    fillNops(instrMem, 12, 64);

    CPU cpu(instrMem, dataMem, 0);

    // x1 written back after 5 cycles, x2 after 6, x3 after 7
    for (int i = 0; i < 7; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();
    CHECK(regs[1] == 5);
    CHECK(regs[2] == 10);
    CHECK(regs[3] == 20);
}

// ============================================================
// Halt behavior (ecall, opcode 0x73)
// ============================================================

TEST_CASE("CPU - Halts on ecall") {
    Memory instrMem(64);
    Memory dataMem(64);

    writeWord(instrMem, 0, 0x00000073); // ecall
    fillNops(instrMem, 4, 64);

    CPU cpu(instrMem, dataMem, 0);

    CHECK(cpu.isHalted() == false);

    // halted flag is set in loadStoreMem(), reading ex_mem.halt —
    // ecall fetched cycle 1, decoded cycle 2, executed cycle 3,
    // reaches loadStoreMem cycle 4
    for (int i = 0; i < 4; i++) {
        cpu.cycle();
    }

    CHECK(cpu.isHalted() == true);
}