#include "../external/doctest.h"
#include "../include/control/control_unit.hpp"
#include <iostream>


// ============================================================
// Constructor / Defaults
// ============================================================

TEST_CASE("ControlUnit - Constructor Defaults") {
    ControlUnit cu;

    FetchSig fs = cu.getFetchSig();
    DecodeSig ds = cu.getDecodeSig();
    ExecuteSig es = cu.getExecuteSig();
    MemorySig ms = cu.getMemorySig();
    WriteBackSig wbs = cu.getWriteBackSig();

    CHECK(fs.PCJ == 0);

    CHECK(ds.SA == 0);
    CHECK(ds.SB == 0);

    CHECK(es.MA == 0);
    CHECK(es.MB == 0);
    CHECK(es.FS == ALUOp::ADD);

    CHECK(ms.MW == false);
    CHECK(ms.MD == 0);
    CHECK(ms.MSN == MemSign::S);
    CHECK(ms.MSZ == MemSize::BYTE);

    CHECK(wbs.LD == false);
    CHECK(wbs.DR == 0);
}


// ============================================================
// R-Type
// ============================================================

TEST_CASE("ControlUnit - ADD") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,       // opcode
        5,          // rd
        6,          // rs1
        7,          // rs2
        0x0,        // funct3
        0x00,       // funct7
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getFetchSig().PCJ == 0);
    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 0);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - SUB") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5,
        6,
        7,
        0x0,
        0x20,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getFetchSig().PCJ == 0);
    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 0);
    CHECK(cu.getExecuteSig().FS == ALUOp::SUB);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - XOR") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x4,
        0x00,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 0);
    CHECK(cu.getExecuteSig().FS == ALUOp::XOR);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - OR") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x6,
        0x00,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 0);
    CHECK(cu.getExecuteSig().FS == ALUOp::OR);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - AND") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x7,
        0x00,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 0);
    CHECK(cu.getExecuteSig().FS == ALUOp::AND);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - SLL") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x1,
        0x00,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 0);
    CHECK(cu.getExecuteSig().FS == ALUOp::SLL);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - SRL") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x5,
        0x00,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SRL);
    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - SRA") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x5,
        0x20,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SRA);
    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - SLT") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x2,
        0x00,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SLT);
    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - SLTU") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::R_type,
        0x33,
        5, 6, 7,
        0x3,
        0x00,
        0
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SLTU);
    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


// ============================================================
// I-Type ALU
// ============================================================

TEST_CASE("ControlUnit - ADDI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5,          // rd
        6,          // rs1
        0,
        0x0,
        0,
        100
    };

    cu.setSigs(instr);

    CHECK(cu.getFetchSig().PCJ == 0);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 0);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - XORI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x4,
        0,
        100
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::XOR);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - ORI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x6,
        0,
        100
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::OR);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - ANDI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x7,
        0,
        100
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::AND);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - SLLI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x1,
        0x00,
        5
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::SLL);

    CHECK(cu.getWriteBackSig().LD == true);
}


TEST_CASE("ControlUnit - SRLI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x5,
        0x00,
        5
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SRL);
    CHECK(cu.getWriteBackSig().LD == true);
}


TEST_CASE("ControlUnit - SRAI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x5,
        0x20,
        0x405
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SRA);
    CHECK(cu.getWriteBackSig().LD == true);
}


TEST_CASE("ControlUnit - SLTI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x2,
        0,
        10
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SLT);
    CHECK(cu.getWriteBackSig().LD == true);
}


TEST_CASE("ControlUnit - SLTIU") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x13,
        5, 6, 0,
        0x3,
        0,
        10
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().FS == ALUOp::SLTU);
    CHECK(cu.getWriteBackSig().LD == true);
}


// ============================================================
// Loads
// ============================================================

TEST_CASE("ControlUnit - LB") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x03,
        5, 6, 0,
        0x0,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 1);
    CHECK(cu.getMemorySig().MSZ == MemSize::BYTE);
    CHECK(cu.getMemorySig().MSN == MemSign::S);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - LBU") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x03,
        5, 6, 0,
        0x4,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 1);
    CHECK(cu.getMemorySig().MSZ == MemSize::BYTE);
    CHECK(cu.getMemorySig().MSN == MemSign::U);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - LH") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x03,
        5, 6, 0,
        0x1,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 1);
    CHECK(cu.getMemorySig().MSZ == MemSize::HALFWORD);
    CHECK(cu.getMemorySig().MSN == MemSign::S);

    CHECK(cu.getWriteBackSig().LD == true);
}


TEST_CASE("ControlUnit - LHU") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x03,
        5, 6, 0,
        0x5,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 1);
    CHECK(cu.getMemorySig().MSZ == MemSize::HALFWORD);
    CHECK(cu.getMemorySig().MSN == MemSign::U);

    CHECK(cu.getWriteBackSig().LD == true);
}


TEST_CASE("ControlUnit - LW") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x03,
        5, 6, 0,
        0x2,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 1);
    CHECK(cu.getMemorySig().MSZ == MemSize::WORD);
    CHECK(cu.getMemorySig().MSN == MemSign::S);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


// ============================================================
// Stores
// ============================================================

TEST_CASE("ControlUnit - SB") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::S_type,
        0x23,
        0, 6, 7,
        0x0,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == true);
    CHECK(cu.getMemorySig().MD == 0);
    CHECK(cu.getMemorySig().MSZ == MemSize::BYTE);

    CHECK(cu.getWriteBackSig().LD == false);
}


TEST_CASE("ControlUnit - SH") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::S_type,
        0x23,
        0, 6, 7,
        0x1,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == true);
    CHECK(cu.getMemorySig().MD == 0);
    CHECK(cu.getMemorySig().MSZ == MemSize::HALFWORD);

    CHECK(cu.getWriteBackSig().LD == false);
}


TEST_CASE("ControlUnit - SW") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::S_type,
        0x23,
        0, 6, 7,
        0x2,
        0,
        4
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 6);
    CHECK(cu.getDecodeSig().SB == 7);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == true);
    CHECK(cu.getMemorySig().MD == 0);
    CHECK(cu.getMemorySig().MSZ == MemSize::WORD);

    CHECK(cu.getWriteBackSig().LD == false);
}


// ============================================================
// Branches
// ============================================================

TEST_CASE("ControlUnit - BEQ") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::B_type,
        0x63,
        0, 5, 6,
        0x0,
        0,
        8
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 5);
    CHECK(cu.getDecodeSig().SB == 6);

    CHECK(cu.getWriteBackSig().LD == false);
    CHECK(cu.getMemorySig().MW == false);
}


TEST_CASE("ControlUnit - BNE") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::B_type,
        0x63,
        0, 5, 6,
        0x1,
        0,
        8
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 5);
    CHECK(cu.getDecodeSig().SB == 6);
    CHECK(cu.getWriteBackSig().LD == false);
}


TEST_CASE("ControlUnit - BLT") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::B_type,
        0x63,
        0, 5, 6,
        0x4,
        0,
        8
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 5);
    CHECK(cu.getDecodeSig().SB == 6);
    CHECK(cu.getWriteBackSig().LD == false);
}


TEST_CASE("ControlUnit - BGE") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::B_type,
        0x63,
        0, 5, 6,
        0x5,
        0,
        8
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 5);
    CHECK(cu.getDecodeSig().SB == 6);
    CHECK(cu.getWriteBackSig().LD == false);
}


TEST_CASE("ControlUnit - BLTU") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::B_type,
        0x63,
        0, 5, 6,
        0x6,
        0,
        8
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 5);
    CHECK(cu.getDecodeSig().SB == 6);
    CHECK(cu.getWriteBackSig().LD == false);
}


TEST_CASE("ControlUnit - BGEU") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::B_type,
        0x63,
        0, 5, 6,
        0x7,
        0,
        8
    };

    cu.setSigs(instr);

    CHECK(cu.getDecodeSig().SA == 5);
    CHECK(cu.getDecodeSig().SB == 6);
    CHECK(cu.getWriteBackSig().LD == false);
}


// ============================================================
// U-Type
// ============================================================

TEST_CASE("ControlUnit - LUI") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::U_type,
        0x37,
        5,
        0,
        0,
        0,
        0,
        0x12345000
    };

    cu.setSigs(instr);

    CHECK(cu.getFetchSig().PCJ == 0);

    CHECK(cu.getExecuteSig().MA == 0);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


TEST_CASE("ControlUnit - AUIPC") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::U_type,
        0x17,
        5,
        0,
        0,
        0,
        0,
        0x12345000
    };

    cu.setSigs(instr);

    CHECK(cu.getFetchSig().PCJ == 0);

    CHECK(cu.getExecuteSig().MA == 1);
    CHECK(cu.getExecuteSig().MB == 1);
    CHECK(cu.getExecuteSig().FS == ALUOp::ADD);

    CHECK(cu.getMemorySig().MW == false);
    CHECK(cu.getMemorySig().MD == 0);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


// ============================================================
// JAL
// ============================================================

TEST_CASE("ControlUnit - JAL") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::J_type,
        0x6F,
        5,
        0,
        0,
        0,
        0,
        16
    };

    cu.setSigs(instr);

    // PC + immediate
    CHECK(cu.getFetchSig().PCJ == 1);

    CHECK(cu.getMemorySig().MW == false);

    // Write PC + 4 to rd
    CHECK(cu.getMemorySig().MD == 2);
    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}


// ============================================================
// JALR
// ============================================================

TEST_CASE("ControlUnit - JALR") {
    ControlUnit cu;

    DecodedInstr instr{
        InstrType::I_type,
        0x67,
        5,
        6,
        0,
        0x0,
        0,
        16
    };

    cu.setSigs(instr);

    // rs1 + immediate
    CHECK(cu.getFetchSig().PCJ == 2);

    CHECK(cu.getDecodeSig().SA == 6);

    CHECK(cu.getMemorySig().MW == false);

    // Write PC + 4 to rd
    CHECK(cu.getMemorySig().MD == 2);

    CHECK(cu.getWriteBackSig().LD == true);
    CHECK(cu.getWriteBackSig().DR == 5);
}