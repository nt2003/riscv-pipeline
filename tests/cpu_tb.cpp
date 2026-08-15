#include "../external/doctest.h"
#include "../include/cpu/cpu.hpp"
#include <iostream>
#include <cstdint>
#include <vector>
#include <bitset>


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

void memoryOutputWordTerminal(Memory& mem, uint32_t addr) {
    std::cout << std::bitset<8>(mem.getContents()[addr+3]) << std::bitset<8>(mem.getContents()[addr+2])
        << std::bitset<8>(mem.getContents()[addr+1]) << std::bitset<8>(mem.getContents()[addr]) << '\n';
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
TEST_CASE("DEBUG - SW/LW address") {
    Memory instrMem(128);
    Memory dataMem(128);

    // addi x1, x0, 16
    writeWord(instrMem, 0, 0x01000093);

    // addi x2, x0, 42
    writeWord(instrMem, 4, 0x02A00113);

    // sw x2, 0(x1)
    writeWord(instrMem, 8, 0x0020A023);

    // lw x3, 0(x1)
    writeWord(instrMem, 12, 0x0000A183);

    fillNops(instrMem, 16, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 12; i++) {
        cpu.cycle();
        
    }
    
    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 16);
    CHECK(regs[2] == 42);
    CHECK(regs[3] == 42);
}

// ============================================================
// Forwarding Unit Tests
// ============================================================
//
// These tests intentionally create RAW (Read-After-Write) hazards.
// The dependent instruction executes before the producer has written
// its result back to the register file, so the forwarding unit must
// provide the newer value from a later pipeline stage.
//
// RISC-V encodings used below:
//   addi rd, rs1, imm
//   add  rd, rs1, rs2
//   sub  rd, rs1, rs2
//
// ============================================================

TEST_CASE("CPU - Forwarding: ALU result forwarded to immediately dependent instruction") {
    Memory instrMem(64);
    Memory dataMem(64);

    // x1 = 10
    // x2 = x1 + x1 = 20
    //
    // x2 depends directly on the result of the immediately preceding
    // instruction. Without EX/MEM forwarding, x2 would see stale x1=0.
    writeWord(instrMem, 0, 0x00A00093);  // addi x1, x0, 10
    writeWord(instrMem, 4, 0x00108133);  // add  x2, x1, x1
    fillNops(instrMem, 8, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 6; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 20);
}


TEST_CASE("CPU - Forwarding: ALU result forwarded through a chain of dependent instructions") {
    Memory instrMem(64);
    Memory dataMem(64);

    // Dependency chain:
    //
    // x1 = 10
    // x2 = x1 + 5  -> 15
    // x3 = x2 + 7  -> 22
    // x4 = x3 + 1  -> 23
    //
    // Every instruction depends on the result of the immediately
    // preceding instruction.

    writeWord(instrMem, 0, 0x00A00093);  // addi x1, x0, 10
    writeWord(instrMem, 4, 0x00508113);  // addi x2, x1, 5
    writeWord(instrMem, 8, 0x00710193);  // addi x3, x2, 7
    writeWord(instrMem, 12, 0x00118213); // addi x4, x3, 1
    fillNops(instrMem, 16, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 15);
    CHECK(regs[3] == 22);
    CHECK(regs[4] == 23);
}


TEST_CASE("CPU - Forwarding: Both ALU operands require forwarding") {
    Memory instrMem(64);
    Memory dataMem(64);

    // x1 = 10
    // x2 = 20
    // x3 = x1 + x2 = 30
    //
    // The ADD needs two recently-produced register values.
    // Both operands must be forwarded rather than read as stale
    // values from the register file.

    writeWord(instrMem, 0, 0x00A00093);  // addi x1, x0, 10
    writeWord(instrMem, 4, 0x01400113);  // addi x2, x0, 20
    writeWord(instrMem, 8, 0x002081B3);  // add  x3, x1, x2
    fillNops(instrMem, 12, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 7; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 20);
    CHECK(regs[3] == 30);
}


TEST_CASE("CPU - Forwarding: Result forwarded when producer is two instructions ahead") {
    Memory instrMem(64);
    Memory dataMem(64);

    // x1 = 10
    // nop
    // x2 = x1 + x1 = 20
    //
    // The producer is two instructions ahead of the consumer.
    // This exercises forwarding from a later pipeline stage than
    // the immediately preceding instruction.

    writeWord(instrMem, 0, 0x00A00093);  // addi x1, x0, 10
    writeWord(instrMem, 4, 0x00000013);  // nop
    writeWord(instrMem, 8, 0x00108133);  // add  x2, x1, x1
    fillNops(instrMem, 12, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 7; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 20);
}


TEST_CASE("CPU - Forwarding: Dependent SUB uses forwarded ALU result") {
    Memory instrMem(64);
    Memory dataMem(64);

    // x1 = 20
    // x2 = 5
    // x3 = x1 - x2 = 15
    //
    // x3 must receive the freshly-produced x1/x2 values rather than
    // stale register-file contents.

    writeWord(instrMem, 0, 0x01400093);  // addi x1, x0, 20
    writeWord(instrMem, 4, 0x00500113);  // addi x2, x0, 5
    writeWord(instrMem, 8, 0x402081B3);  // sub  x3, x1, x2
    fillNops(instrMem, 12, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 7; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 20);
    CHECK(regs[2] == 5);
    CHECK(regs[3] == 15);
}


TEST_CASE("CPU - Forwarding: Longer dependency chain does not use stale values") {
    Memory instrMem(64);
    Memory dataMem(64);

    // x1 = 5
    // x2 = x1 + x1 = 10
    // x3 = x2 + x1 = 15
    // x4 = x3 + x2 = 25
    // x5 = x4 + x3 = 40
    //
    // This creates a sustained chain of RAW hazards. At several
    // points, the operands required by an instruction are still
    // somewhere in the pipeline rather than in the register file.

    writeWord(instrMem, 0,  0x00500093); // addi x1, x0, 5
    writeWord(instrMem, 4,  0x00108133); // add  x2, x1, x1
    writeWord(instrMem, 8,  0x001101B3); // add  x3, x2, x1
    writeWord(instrMem, 12, 0x00218233); // add  x4, x3, x2
    writeWord(instrMem, 16, 0x003202B3); // add  x5, x4, x3
    fillNops(instrMem, 20, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 9; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 5);
    CHECK(regs[2] == 10);
    CHECK(regs[3] == 15);
    CHECK(regs[4] == 25);
    CHECK(regs[5] == 40);
}


TEST_CASE("CPU - Forwarding: Destination register is not corrupted by subsequent dependencies") {
    Memory instrMem(64);
    Memory dataMem(64);

    // x1 = 7
    // x2 = x1 + 3 = 10
    // x3 = x2 + 4 = 14
    // x1 = x3 + 6 = 20
    // x4 = x1 + x2 = 30
    //
    // This also verifies that forwarding follows the newest value
    // for a register after that register is written again.

    writeWord(instrMem, 0,  0x00700093); // addi x1, x0, 7
    writeWord(instrMem, 4,  0x00308113); // addi x2, x1, 3
    writeWord(instrMem, 8,  0x00410193); // addi x3, x2, 4
    writeWord(instrMem, 12, 0x00618093); // addi x1, x3, 6
    writeWord(instrMem, 16, 0x00208233); // add  x4, x1, x2
    fillNops(instrMem, 20, 64);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 9; i++) {
        cpu.cycle();
    }

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 20);
    CHECK(regs[2] == 10);
    CHECK(regs[3] == 14);
    CHECK(regs[4] == 30);
}

// ============================================================
// RV32I FULL COVERAGE TEST SUITE
// ============================================================
//
// Coverage:
//
//   U-type:
//       LUI
//       AUIPC
//
//   Jumps:
//       JAL
//       JALR
//
//   Branches:
//       BEQ
//       BNE
//       BLT
//       BGE
//       BLTU
//       BGEU
//
//   Loads:
//       LB
//       LH
//       LW
//       LBU
//       LHU
//
//   Stores:
//       SB
//       SH
//       SW
//
//   Immediate ALU:
//       ADDI
//       SLTI
//       SLTIU
//       XORI
//       ORI
//       ANDI
//       SLLI
//       SRLI
//       SRAI
//
//   Register ALU:
//       ADD
//       SUB
//       SLL
//       SLT
//       SLTU
//       XOR
//       SRL
//       SRA
//       OR
//       AND
//
//   Pipeline / hazard coverage:
//       RAW hazards
//       EX -> EX forwarding
//       MEM -> EX forwarding
//       forwarding to both operands
//       forwarding into branches
//       forwarding into JALR
//       load-use hazards
//       taken-branch flushing
//       JAL flushing
//       JALR flushing
//       dependency chains
//       x0 hazards
//
// ============================================================


// ============================================================
// RV32I Encoding Helpers
// ============================================================

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


static uint32_t rv32_encodeShiftI(
    uint32_t funct7,
    uint32_t shamt,
    uint32_t rs1,
    uint32_t funct3,
    uint32_t rd
) {
    return (funct7 << 25) |
           ((shamt & 0x1F) << 20) |
           (rs1 << 15) |
           (funct3 << 12) |
           (rd << 7) |
           0x13;
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


static uint32_t rv32_encodeU(
    uint32_t imm20,
    uint32_t rd,
    uint32_t opcode
) {
    return ((imm20 & 0xFFFFF) << 12) |
           (rd << 7) |
           opcode;
}


static uint32_t rv32_encodeJ(
    int32_t imm,
    uint32_t rd
) {
    uint32_t uimm = static_cast<uint32_t>(imm);

    uint32_t bit20     = (uimm >> 20) & 0x1;
    uint32_t bits10_1  = (uimm >> 1) & 0x3FF;
    uint32_t bit11     = (uimm >> 11) & 0x1;
    uint32_t bits19_12 = (uimm >> 12) & 0xFF;

    return (bit20 << 31) |
           (bits19_12 << 12) |
           (bit11 << 20) |
           (bits10_1 << 21) |
           (rd << 7) |
           0x6F;
}


static uint32_t rv32_lui(uint32_t rd, uint32_t imm20) {
    return rv32_encodeU(imm20, rd, 0x37);
}


static uint32_t rv32_auipc(uint32_t rd, uint32_t imm20) {
    return rv32_encodeU(imm20, rd, 0x17);
}


// ============================================================
// U-TYPE INSTRUCTIONS
// ============================================================

TEST_CASE("RV32I - LUI") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_lui(1, 0x12345));
    fillNops(instrMem, 4, 128);

    CPU cpu(instrMem, dataMem, 0);
    for (int i = 0; i < 8; i++) 
        cpu.cycle();

    auto regs = cpu.getRegFile();
    CHECK(regs[1] == 0x12345000);
}


TEST_CASE("RV32I - LUI followed by dependent ADDI") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 0x12345000
    // x2 = x1 + 5

    writeWord(instrMem, 0, rv32_lui(1, 0x12345));
    writeWord(instrMem, 4, rv32_encodeI(5, 1, 0x0, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 0x12345000);
    CHECK(regs[2] == 0x12345005);
}


TEST_CASE("RV32I - AUIPC") {
    Memory instrMem(128);
    Memory dataMem(64);

    // AUIPC at PC=0.
    // Result = PC + immediate.

    writeWord(instrMem, 0, rv32_auipc(1, 0x12345));
    fillNops(instrMem, 4, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 0x12345000);
}


TEST_CASE("RV32I - AUIPC at nonzero PC") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, 0x00000013); // nop
    writeWord(instrMem, 4, rv32_auipc(1, 0x00001));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    // PC = 4, immediate = 0x1000
    CHECK(regs[1] == 0x1004);
}


// ============================================================
// IMMEDIATE ALU INSTRUCTIONS
// ============================================================

TEST_CASE("RV32I - ADDI") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(42, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(-7, 1, 0x0, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 42);
    CHECK(regs[2] == 35);
}


TEST_CASE("RV32I - SLTI signed comparison") {
    Memory instrMem(128);
    Memory dataMem(64);

    // -1 < 1 => true

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(1, 1, 0x2, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 0xFFFFFFFF);
    CHECK(regs[2] == 1);
}


TEST_CASE("RV32I - SLTIU unsigned comparison") {
    Memory instrMem(128);
    Memory dataMem(64);

    // 0xFFFFFFFF < 1 unsigned => false

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(1, 1, 0x3, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 0xFFFFFFFF);
    CHECK(regs[2] == 0);
}


TEST_CASE("RV32I - XORI") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x0F0, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0x0FF, 1, 0x4, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0x0F);
}


TEST_CASE("RV32I - ORI") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x0F0, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0x00F, 1, 0x6, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0xFF);
}


TEST_CASE("RV32I - ANDI") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x0FF, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0x00F, 1, 0x7, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0x0F);
}


// ============================================================
// SHIFT IMMEDIATE
// ============================================================

TEST_CASE("RV32I - SLLI") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeShiftI(0x00, 4, 1, 0x1, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 16);
}


TEST_CASE("RV32I - SRLI") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeShiftI(0x00, 4, 1, 0x5, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0x0FFFFFFF);
}


TEST_CASE("RV32I - SRAI sign extension") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(-16, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeShiftI(0x20, 2, 1, 0x5, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0xFFFFFFFC);
}


// ============================================================
// REGISTER-REGISTER ALU
// ============================================================

TEST_CASE("RV32I - ADD") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(20, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(22, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x0, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 42);
}


TEST_CASE("RV32I - SUB") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(20, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(7, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x20, 2, 1, 0x0, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 13);
}


TEST_CASE("RV32I - SLL") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(5, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x1, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 32);
}


TEST_CASE("RV32I - SLT signed") {
    Memory instrMem(128);
    Memory dataMem(64);

    // -1 < 1

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(1, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x2, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 1);
}


TEST_CASE("RV32I - SLTU unsigned") {
    Memory instrMem(128);
    Memory dataMem(64);

    // 0xFFFFFFFF < 1 unsigned => false

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(1, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x3, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 0);
}


TEST_CASE("RV32I - XOR") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x0F0, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0x0FF, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x4, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 0x0F);
}


TEST_CASE("RV32I - SRL") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(4, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x5, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 0x0FFFFFFF);
}


TEST_CASE("RV32I - SRA") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(-16, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(2, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x20, 2, 1, 0x5, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 0xFFFFFFFC);
}


TEST_CASE("RV32I - OR") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x0F0, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0x00F, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x6, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 0xFF);
}


TEST_CASE("RV32I - AND") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x0FF, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0x00F, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x7, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 0x0F);
}


// ============================================================
// SHIFT EDGE CASES
// ============================================================

TEST_CASE("RV32I - Shift amount 0") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x123, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeShiftI(0x00, 0, 1, 0x1, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0x123);
}


TEST_CASE("RV32I - Shift amount 31") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeShiftI(0x00, 31, 1, 0x1, 2));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 8; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0x80000000);
}


// ============================================================
// MEMORY INSTRUCTIONS
// ============================================================

TEST_CASE("RV32I - SW followed by LW") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 0x12345678
    //
    // Build value using LUI + ADDI.
    // Store it and immediately load it back.

    writeWord(instrMem, 0,  rv32_lui(1, 0x12345));
    writeWord(instrMem, 4,  rv32_encodeI(0x678, 1, 0x0, 1));
    writeWord(instrMem, 8,  rv32_encodeS(0, 1, 0, 0x2)); // sw x1,0(x0)
    writeWord(instrMem, 12, rv32_encodeI(0, 0, 0x2, 2, 0x03)); // lw x2,0(x0)

    fillNops(instrMem, 16, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 0x12345678);
    CHECK(regs[2] == 0x12345678);
}


TEST_CASE("RV32I - SB and LBU") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(0x7F, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeS(0, 1, 0, 0x0)); // sb
    writeWord(instrMem, 8, rv32_encodeI(0, 0, 0x4, 2, 0x03)); // lbu

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 12; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0x7F);
}


TEST_CASE("RV32I - SB sign extension with LB") {
    Memory instrMem(128);
    Memory dataMem(64);

    // Store 0xFF.
    // LB must sign extend to 0xFFFFFFFF.
    // LBU must produce 0x000000FF.

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeS(0, 1, 0, 0x0));
    writeWord(instrMem, 8, rv32_encodeI(0, 0, 0x0, 2, 0x03)); // lb
    writeWord(instrMem, 12, rv32_encodeI(0, 0, 0x4, 3, 0x03)); // lbu

    fillNops(instrMem, 16, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0xFFFFFFFF);
    CHECK(regs[3] == 0x000000FF);
}


TEST_CASE("RV32I - SH and LHU") {
    Memory instrMem(128);
    Memory dataMem(64);

    // Store 0xFFFF.
    // LHU should produce 0x0000FFFF.

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeS(4, 1, 0, 0x1)); // sh x1,4(x0)
    writeWord(instrMem, 8, rv32_encodeI(4, 0, 0x5, 2, 0x03)); // lhu

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 14; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0x0000FFFF);
}


TEST_CASE("RV32I - LH sign extension") {
    Memory instrMem(128);
    Memory dataMem(64);

    // Store -2 as a halfword.
    // LH must produce 0xFFFFFFFE.

    writeWord(instrMem, 0, rv32_encodeI(-2, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeS(0, 1, 0, 0x1));
    writeWord(instrMem, 8, rv32_encodeI(0, 0, 0x1, 2, 0x03)); // lh

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 14; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 0xFFFFFFFE);
}


// ============================================================
// STORE DATA HAZARD
// ============================================================

TEST_CASE("RV32I - Store consumes freshly produced value") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 42
    // sw x1,0(x0)
    // lw x2,0(x0)
    //
    // Store data must receive the current x1 rather than stale data.

    writeWord(instrMem, 0,  rv32_encodeI(42, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeS(0, 1, 0, 0x2));
    writeWord(instrMem, 8,  rv32_encodeI(0, 0, 0x2, 2, 0x03));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 14; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 42);
    CHECK(regs[2] == 42);
}


// ============================================================
// LOAD-USE HAZARD
// ============================================================

TEST_CASE("RV32I - Load followed immediately by dependent ADDI") {
    Memory instrMem(128);
    Memory dataMem(64);

    // Put 10 in memory using CPU instructions.
    writeWord(instrMem, 0,  rv32_encodeI(10, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeS(0, 1, 0, 0x2));

    // Load and immediately consume.
    writeWord(instrMem, 8,  rv32_encodeI(0, 0, 0x2, 2, 0x03));
    writeWord(instrMem, 12, rv32_encodeI(5, 2, 0x0, 3));

    fillNops(instrMem, 16, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 18; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 10);
    CHECK(regs[3] == 15);
}


TEST_CASE("RV32I - Load-use hazard with two dependent consumers") {
    Memory instrMem(128);
    Memory dataMem(64);

    // Store 7.
    writeWord(instrMem, 0,  rv32_encodeI(7, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeS(0, 1, 0, 0x2));

    // Load.
    writeWord(instrMem, 8,  rv32_encodeI(0, 0, 0x2, 2, 0x03));

    // x3 = x2 + 1 = 8
    // x4 = x3 + 1 = 9
    writeWord(instrMem, 12, rv32_encodeI(1, 2, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(1, 3, 0x0, 4));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 20; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[2] == 7);
    CHECK(regs[3] == 8);
    CHECK(regs[4] == 9);
}


// ============================================================
// FORWARDING
// ============================================================

TEST_CASE("RV32I - Forwarding: EX result consumed immediately") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(10, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(5, 1, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeI(3, 2, 0x0, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 12; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 15);
    CHECK(regs[3] == 18);
}


TEST_CASE("RV32I - Forwarding: Both ALU operands") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(10, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(20, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 2, 1, 0x0, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 12; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 30);
}


TEST_CASE("RV32I - Forwarding: Long dependency chain") {
    Memory instrMem(128);
    Memory dataMem(64);

    // 1 -> 2 -> 4 -> 8 -> 16 -> 32

    writeWord(instrMem, 0,  rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeR(0x00, 1, 1, 0x0, 2));
    writeWord(instrMem, 8,  rv32_encodeR(0x00, 2, 2, 0x0, 3));
    writeWord(instrMem, 12, rv32_encodeR(0x00, 3, 3, 0x0, 4));
    writeWord(instrMem, 16, rv32_encodeR(0x00, 4, 4, 0x0, 5));
    writeWord(instrMem, 20, rv32_encodeR(0x00, 5, 5, 0x0, 6));

    fillNops(instrMem, 24, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 18; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 1);
    CHECK(regs[2] == 2);
    CHECK(regs[3] == 4);
    CHECK(regs[4] == 8);
    CHECK(regs[5] == 16);
    CHECK(regs[6] == 32);
}


// ============================================================
// BRANCHES
// ============================================================

TEST_CASE("RV32I - BEQ taken") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(5, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(5, 0, 0x0, 2));

    // Skip PC 12 and land at PC 16.
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x0));

    writeWord(instrMem, 12, rv32_encodeI(1, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(99, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 99);
}


TEST_CASE("RV32I - BEQ not taken") {
    Memory instrMem(128);
    Memory dataMem(64);
    writeWord(instrMem, 0,  rv32_encodeI(5, 0, 0x0, 1));   // x1 = 5
    writeWord(instrMem, 4,  rv32_encodeI(6, 0, 0x0, 2));   // x2 = 6
    writeWord(instrMem, 8,  rv32_encodeB(8, 2, 1, 0x0));   // beq x1,x2,8
    writeWord(instrMem, 12, rv32_encodeI(42, 0, 0x0, 3));  // x3 = 42 (should run if not taken)
    writeWord(instrMem, 16, rv32_encodeI(99, 0, 0x0, 4));  // x4 = 99 (rd changed to x4)
    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);
    
    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 42);
}


TEST_CASE("RV32I - BNE taken") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(5, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(6, 0, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x1));

    writeWord(instrMem, 12, rv32_encodeI(1, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(55, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 55);
}


TEST_CASE("RV32I - BLT signed") {
    Memory instrMem(128);
    Memory dataMem(64);

    // -1 < 1 signed.

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(1, 0, 0x0, 2));

    // BLT funct3 = 100
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x4));

    writeWord(instrMem, 12, rv32_encodeI(1, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(77, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 77);
}


TEST_CASE("RV32I - BGE signed") {
    Memory instrMem(128);
    Memory dataMem(64);

    // 1 >= -1 signed.

    writeWord(instrMem, 0, rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(-1, 0, 0x0, 2));

    // BGE funct3 = 101
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x5));

    writeWord(instrMem, 12, rv32_encodeI(1, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(88, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 88);
}


TEST_CASE("RV32I - BLTU unsigned") {
    Memory instrMem(128);
    Memory dataMem(64);

    // 1 < 0xFFFFFFFF unsigned.

    writeWord(instrMem, 0, rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(-1, 0, 0x0, 2));

    // BLTU funct3 = 110
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x6));

    writeWord(instrMem, 12, rv32_encodeI(1, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(66, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 66);
}


TEST_CASE("RV32I - BGEU unsigned") {
    Memory instrMem(128);
    Memory dataMem(64);

    // 0xFFFFFFFF >= 1 unsigned.

    writeWord(instrMem, 0, rv32_encodeI(-1, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(1, 0, 0x0, 2));

    // BGEU funct3 = 111
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x7));

    writeWord(instrMem, 12, rv32_encodeI(1, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(123, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[3] == 123);
}


// ============================================================
// BRANCH + FORWARDING
// ============================================================

TEST_CASE("RV32I - Branch consumes forwarded result") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 10
    // x2 = x1
    // BEQ x1,x2
    //
    // Tests forwarding into branch comparison.

    writeWord(instrMem, 0, rv32_encodeI(10, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0, 1, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x0));

    writeWord(instrMem, 12, rv32_encodeI(1, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(77, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 18; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 10);
    CHECK(regs[3] == 77);
}


// ============================================================
// JAL
// ============================================================

TEST_CASE("RV32I - JAL writes PC+4") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeJ(8, 1));
    writeWord(instrMem, 4, rv32_encodeI(99, 0, 0x0, 2)); // wrong path
    writeWord(instrMem, 8, rv32_encodeI(42, 0, 0x0, 3));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 14; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 4);
    CHECK(regs[2] == 0);
    CHECK(regs[3] == 42);
}


TEST_CASE("RV32I - JAL flushes multiple wrong-path instructions") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeJ(16, 1));

    writeWord(instrMem, 4,  rv32_encodeI(1, 0, 0x0, 2));
    writeWord(instrMem, 8,  rv32_encodeI(2, 0, 0x0, 2));
    writeWord(instrMem, 12, rv32_encodeI(3, 0, 0x0, 2));

    writeWord(instrMem, 16, rv32_encodeI(42, 0, 0x0, 2));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 16; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 4);
    CHECK(regs[2] == 42);
}


// ============================================================
// JALR
// ============================================================

TEST_CASE("RV32I - JALR") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 16
    // JALR x5, x1, 0
    // target = PC 16

    writeWord(instrMem, 0, rv32_encodeI(16, 0, 0x0, 1));

    // jalr funct3=000, opcode=1100111
    writeWord(instrMem, 4, rv32_encodeI(0, 1, 0x0, 5, 0x67));

    writeWord(instrMem, 8,  rv32_encodeI(99, 0, 0x0, 2));
    writeWord(instrMem, 12, rv32_encodeI(98, 0, 0x0, 2));
    writeWord(instrMem, 16, rv32_encodeI(42, 0, 0x0, 2));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 18; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 16);
    CHECK(regs[5] == 8);
    CHECK(regs[2] == 42);
}


TEST_CASE("RV32I - JALR consumes forwarded target address") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 16
    // JALR immediately uses x1.
    //
    // Tests forwarding into JALR target calculation.

    writeWord(instrMem, 0, rv32_encodeI(16, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0, 1, 0x0, 5, 0x67));

    writeWord(instrMem, 8,  rv32_encodeI(99, 0, 0x0, 2));
    writeWord(instrMem, 12, rv32_encodeI(98, 0, 0x0, 2));
    writeWord(instrMem, 16, rv32_encodeI(77, 0, 0x0, 2));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 18; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 16);
    CHECK(regs[5] == 8);
    CHECK(regs[2] == 77);
}


// ============================================================
// CONTROL HAZARD + DATA HAZARD COMBINATION
// ============================================================

TEST_CASE("RV32I - Taken branch flush plus forwarding") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 5
    // x2 = x1
    // branch x1 == x2
    // wrong path attempts x3 = 111
    // target x3 = 222

    writeWord(instrMem, 0, rv32_encodeI(5, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0, 1, 0x0, 2));
    writeWord(instrMem, 8, rv32_encodeB(8, 2, 1, 0x0));

    writeWord(instrMem, 12, rv32_encodeI(111, 0, 0x0, 3));
    writeWord(instrMem, 16, rv32_encodeI(222, 0, 0x0, 3));

    fillNops(instrMem, 20, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 18; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 5);
    CHECK(regs[2] == 5);
    CHECK(regs[3] == 222);
}


TEST_CASE("RV32I - JALR flush plus dependency") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 16
    // JALR x5,x1
    // wrong path must disappear
    // target computes dependent result

    writeWord(instrMem, 0, rv32_encodeI(16, 0, 0x0, 1));
    writeWord(instrMem, 4, rv32_encodeI(0, 1, 0x0, 5, 0x67));

    writeWord(instrMem, 8,  rv32_encodeI(111, 0, 0x0, 2));
    writeWord(instrMem, 12, rv32_encodeI(222, 0, 0x0, 2));
    writeWord(instrMem, 16, rv32_encodeI(7, 0, 0x0, 2));
    writeWord(instrMem, 20, rv32_encodeI(5, 2, 0x0, 3));

    fillNops(instrMem, 24, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 20; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 16);
    CHECK(regs[5] == 8);
    CHECK(regs[2] == 7);
    CHECK(regs[3] == 12);
}


// ============================================================
// x0 / HAZARD EDGE CASES
// ============================================================

TEST_CASE("RV32I - x0 always remains zero") {
    Memory instrMem(128);
    Memory dataMem(64);

    writeWord(instrMem, 0, rv32_encodeI(123, 0, 0x0, 0));
    writeWord(instrMem, 4, rv32_encodeI(456, 0, 0x0, 0));
    writeWord(instrMem, 8, rv32_encodeR(0x00, 0, 0, 0x0, 0));

    fillNops(instrMem, 12, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 12; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[0] == 0);
}


TEST_CASE("RV32I - Dependency on x0 sees zero") {
    Memory instrMem(128);
    Memory dataMem(64);

    // Attempt to poison x0.
    writeWord(instrMem, 0, rv32_encodeI(123, 0, 0x0, 0));

    // Must still use zero.
    writeWord(instrMem, 4, rv32_encodeI(5, 0, 0x0, 1));

    fillNops(instrMem, 8, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 10; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[0] == 0);
    CHECK(regs[1] == 5);
}


// ============================================================
// OVERWRITE HAZARD
// ============================================================

TEST_CASE("RV32I - Register overwritten multiple times") {
    Memory instrMem(128);
    Memory dataMem(64);

    // x1 = 1
    // x1 = 2
    // x1 = 3
    // x2 = x1
    //
    // x2 must receive the newest x1.

    writeWord(instrMem, 0,  rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeI(2, 0, 0x0, 1));
    writeWord(instrMem, 8,  rv32_encodeI(3, 0, 0x0, 1));
    writeWord(instrMem, 12, rv32_encodeI(0, 1, 0x0, 2));

    fillNops(instrMem, 16, 128);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 14; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 3);
    CHECK(regs[2] == 3);
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

    for (int i = 0; i < 30; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 10);
    CHECK(regs[2] == 20);
    CHECK(regs[3] == 30);
    CHECK(regs[4] == 30);
    CHECK(regs[5] == 42);
    CHECK(regs[6] == 42);

    // Proves branch was taken AND wrong path was flushed.
    CHECK(regs[7] == 99);
}


// ============================================================
// LARGE DEPENDENCY STRESS TEST
// ============================================================

TEST_CASE("RV32I - Large dependency chain with mixed operations") {
    Memory instrMem(256);
    Memory dataMem(128);

    // x1 = 5
    // x2 = x1 + 5 = 10
    // x3 = x2 - 3 = 7
    // x4 = x3 << 2 = 28
    // x5 = x4 | 3 = 31
    // x6 = x5 & 15 = 15
    // x7 = x6 ^ 10 = 5
    // x8 = x7 + x1 = 10
    // x9 = x8 - x3 = 3

    writeWord(instrMem, 0,  rv32_encodeI(5, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeI(5, 1, 0x0, 2));

    writeWord(instrMem, 8,  rv32_encodeI(-3, 2, 0x0, 3));

    writeWord(instrMem, 12, rv32_encodeShiftI(0x00, 2, 3, 0x1, 4));

    writeWord(instrMem, 16, rv32_encodeI(3, 4, 0x6, 5));
    writeWord(instrMem, 20, rv32_encodeI(15, 5, 0x7, 6));
    writeWord(instrMem, 24, rv32_encodeI(10, 6, 0x4, 7));

    writeWord(instrMem, 28, rv32_encodeR(0x00, 1, 7, 0x0, 8));
    writeWord(instrMem, 32, rv32_encodeR(0x20, 3, 8, 0x0, 9));

    fillNops(instrMem, 36, 256);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 24; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[1] == 5);
    CHECK(regs[2] == 10);
    CHECK(regs[3] == 7);
    CHECK(regs[4] == 28);
    CHECK(regs[5] == 31);
    CHECK(regs[6] == 15);
    CHECK(regs[7] == 5);
    CHECK(regs[8] == 10);
    CHECK(regs[9] == 3);
}


// ============================================================
// FINAL ARCHITECTURAL SANITY TEST
// ============================================================

TEST_CASE("RV32I - Final register file sanity") {
    Memory instrMem(256);
    Memory dataMem(128);

    // Populate a variety of registers and ensure unrelated
    // registers remain untouched.

    writeWord(instrMem, 0,  rv32_encodeI(1, 0, 0x0, 1));
    writeWord(instrMem, 4,  rv32_encodeI(2, 0, 0x0, 2));
    writeWord(instrMem, 8,  rv32_encodeI(3, 0, 0x0, 3));
    writeWord(instrMem, 12, rv32_encodeR(0x00, 2, 1, 0x0, 4));
    writeWord(instrMem, 16, rv32_encodeR(0x00, 3, 4, 0x0, 5));

    fillNops(instrMem, 20, 256);

    CPU cpu(instrMem, dataMem, 0);

    for (int i = 0; i < 20; i++)
        cpu.cycle();

    auto regs = cpu.getRegFile();

    CHECK(regs[0] == 0);
    CHECK(regs[1] == 1);
    CHECK(regs[2] == 2);
    CHECK(regs[3] == 3);
    CHECK(regs[4] == 3);
    CHECK(regs[5] == 6);

    // Registers not explicitly written should remain zero.
    CHECK(regs[6] == 0);
    CHECK(regs[7] == 0);
    CHECK(regs[8] == 0);
    CHECK(regs[9] == 0);
    CHECK(regs[10] == 0);
    CHECK(regs[11] == 0);
    CHECK(regs[12] == 0);
    CHECK(regs[13] == 0);
    CHECK(regs[14] == 0);
    CHECK(regs[15] == 0);
}