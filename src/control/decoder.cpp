#include "../../include/control/decoder.hpp"
#include <stdexcept>
#include <iostream>

uint32_t extractOpcode(uint32_t instr) {
    return instr&0x7F;
}
uint32_t extractRd(uint32_t instr) {
    return (instr&0xF80)>>7;
}
uint32_t extractRs1(uint32_t instr) {
    return (instr&0xF8000)>>15;
}
uint32_t extractRs2(uint32_t instr) {
    return (instr&0x1F00000)>>20;
}
uint32_t extractFunct3(uint32_t instr) {
    return (instr&0x7000)>>12;
}
uint32_t extractFunct7(uint32_t instr) {
    return (instr&0xFE000000)>>25;
}
InstrType getType(uint32_t instr) {
    uint32_t opcode = instr & 0x7F;

    switch(opcode) {
        case 0x33:
            return InstrType::R_type;

        case 0x13:
        case 0x03:
        case 0x67:
        case 0x73:
            return InstrType::I_type;

        case 0x23:
            return InstrType::S_type;

        case 0x63:
            return InstrType::B_type;

        case 0x37:
        case 0x17:
            return InstrType::U_type;

        case 0x6F:
            return InstrType::J_type;

        default:
            std::cout << opcode << '\n';
            throw std::logic_error("Unknown opcode - getType");
    }
}


uint32_t extractImm(uint32_t instr, InstrType type) {
    switch(type) {
        case InstrType::R_type:{
            return 0x0;
        }
        case InstrType::I_type:{
            return static_cast<uint32_t>(static_cast<int32_t>((instr&0xFFF00000))>>20);
        }
        case InstrType::S_type:{
            uint32_t lo = (instr&0xF80)>>7;
            uint32_t hi = static_cast<uint32_t>(
                            static_cast<int32_t>((instr&0xFE000000))>>20);
            return hi+lo;
        }
        case InstrType::B_type:{
            uint32_t b12 = static_cast<uint32_t>(
                            (static_cast<int32_t>(instr&0x80000000))>>19);
            uint32_t b11 = (instr&0x80)<<4;
            uint32_t lo = (instr&0xF00)>>7;
            uint32_t hi = (instr&0x7E000000)>>20;
            return hi+lo+b12+b11;
        }
        case InstrType::U_type:{
            return instr&0xFFFFF000; 
        }
        case InstrType::J_type:{
            uint32_t b20 = static_cast<uint32_t>(
                            (static_cast<int32_t>(instr&0x80000000))>>11);
            uint32_t b11 = (instr&0x100000)>>9;
            uint32_t lo = (instr&0x7FE00000)>>20;
            uint32_t hi = instr&0xFF000;
            return hi+lo+b20+b11;
        }
        default:{
            throw std::logic_error("Unknown instruction type - likely opcode error");
        }
    }
}

DecodedInstr decodeInstr(uint32_t instr) {
    DecodedInstr di;

    di.opcode = extractOpcode(instr);
    di.rd = extractRd(instr);
    di.rs1 = extractRs1(instr);
    di.rs2 = extractRs2(instr);
    di.funct3 = extractFunct3(instr);
    di.funct7 = extractFunct7(instr);
    di.type = getType(instr);
    di.imm = extractImm(instr, di.type);

    return di;
}

