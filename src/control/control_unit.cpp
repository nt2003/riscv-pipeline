#include "../../include/control/control_unit.hpp"
#include <stdexcept>
#include <iostream>

ControlUnit::ControlUnit() {};

void ControlUnit::setSigs(DecodedInstr instr) {
    fs.PCJ = 0;
    switch(instr.opcode) {
        case 0x33: {
            fs.PCJ = 0x0;

            ds.SA = instr.rs1;
            ds.SB = instr.rs2;

            es.MA = 0x0;
            es.MB = 0x0;
            switch(instr.funct3) {
                case 0x0: {
                    if (instr.funct7 == 0x0) {
                        es.FS = ALUOp::ADD;
                    } else {
                        es.FS = ALUOp::SUB;
                    }
                    break;
                }
                case 0x1: {
                    es.FS = ALUOp::SLL;
                    break;
                }
                case 0x2: {
                    es.FS = ALUOp::SLT;
                    break;
                }
                case 0x3: {
                    es.FS = ALUOp::SLTU;
                    break;
                }
                case 0x4: {
                    es.FS = ALUOp::XOR;
                    break;
                }
                case 0x5: {
                    if (instr.funct7 == 0) {
                        es.FS = ALUOp::SRL;
                    } else {
                        es.FS = ALUOp::SRA;
                    }
                    break;
                }
                case 0x6: {
                    es.FS = ALUOp::OR;
                    break;
                }
                case 0x7: {
                    es.FS = ALUOp::AND;
                    break;
                }
                default: {
                    throw std::logic_error("Unknown funct3/opcode - Reg/Reg instr");
                }
            }
            
            ms.MW = false;
            ms.MD = 0x0;

            wbs.LD = true;
            wbs.DR = instr.rd;
            
            break;
        }
        case 0x13: {
            fs.PCJ = 0x0;

            ds.SA = instr.rs1;

            es.MA = 0x0;
            es.MB = 0x1;
            switch(instr.funct3) {
                case 0x0: {
                    es.FS = ALUOp::ADD;
                    break;
                }
                case 0x1: {
                    es.FS = ALUOp::SLL;
                    break;
                }
                case 0x2: {
                    es.FS = ALUOp::SLT;
                    break;
                }
                case 0x3: {
                    es.FS = ALUOp::SLTU;
                    break;
                }
                case 0x4: {
                    es.FS = ALUOp::XOR;
                    break;
                }
                case 0x5: {
                    if ((instr.imm>>5) == 0) {
                        es.FS = ALUOp::SRL;
                    } else {
                        es.FS = ALUOp::SRA;
                    }
                    break;
                }
                case 0x6: {
                    es.FS = ALUOp::OR;
                    break;
                }
                case 0x7: {
                    es.FS = ALUOp::AND;
                    break;
                }
                default:
                    throw std::logic_error("Unknown funct3 - IMM to Reg Instr");
                    break;
            }
            
            ms.MW = false;
            ms.MD = 0x0;

            wbs.LD = true;
            wbs.DR = instr.rd;
            
            break;
        }
        case 0x3: {
            fs.PCJ = 0x0;
            
            ds.SA = instr.rs1;
            
            es.MA = 0x0;
            es.MB = 0x1;
            es.FS = ALUOp::ADD;

            ms.MW = false;
            ms.MD = 0x1;
            switch(instr.funct3) {
                case 0x0: {
                    ms.MSN = MemSign::S;
                    ms.MSZ = MemSize::BYTE;
                    break;
                }
                case 0x1: {
                    ms.MSN = MemSign::S;
                    ms.MSZ = MemSize::HALFWORD;
                    break;
                }
                case 0x2: {
                    ms.MSN = MemSign::S;
                    ms.MSZ = MemSize::WORD;
                    break;
                }
                case 0x4: {
                    ms.MSN = MemSign::U;
                    ms.MSZ = MemSize::BYTE;
                    break;
                }
                case 0x5: {
                    ms.MSN = MemSign::U;
                    ms.MSZ = MemSize::HALFWORD;
                    break;
                }
                default: {
                    throw std::logic_error("Unknown funct3 - Load Instr");
                }
            }
            
            wbs.LD = true;
            wbs.DR = instr.rd;
            
            break;
        }
        case 0x23: {
            fs.PCJ = 0x0;

            ds.SA = instr.rs1;
            ds.SB = instr.rs2;

            es.MA = 0x0;
            es.MB = 0x1;
            es.FS = ALUOp::ADD;

            ms.MW = true;
            switch(instr.funct3) {
                case 0x0: {
                    ms.MSZ = MemSize::BYTE;
                    break;
                }
                case 0x1: {
                    ms.MSZ = MemSize::HALFWORD;
                    break;
                }
                case 0x2: {
                    ms.MSZ = MemSize::WORD;
                    break;
                }
                default: {
                    throw std::logic_error("Unknown funct3 - S-type instr");
                }
            }
            
            wbs.LD = false;
            
            break;
        }
        case 0x63: {
            ds.SA = instr.rs1;
            ds.SB = instr.rs2;

            es.MA = 0x0;
            es.MB = 0x0;

            ms.MW = false;

            wbs.LD = false;
            
            switch(instr.funct3) {
                case 0x0: {
                    es.FS = ALUOp::SUB;
                    break;
                }
                case 0x1: {
                    es.FS = ALUOp::SUB;
                    break;
                }
                case 0x4: {
                    es.FS = ALUOp::SLT;
                    break;
                }
                case 0x5: {
                    es.FS = ALUOp::SLT;
                    break;
                }
                case 0x6: {
                    es.FS = ALUOp::SLTU;
                    break;
                }
                case 0x7: {
                    es.FS = ALUOp::SLTU;
                    break;
                }
                default: {
                    throw std::logic_error("Unknow funct3 - B-type instr");
                }
            }
            
            break;
        }
        case 0x6F: {
            fs.PCJ = 0x1;

            ms.MD = 0x2;
            ms.MW = false;

            wbs.LD = true;
            wbs.DR = instr.rd;

            break;
        }
        case 0x67: {
            fs.PCJ = 0x2;
            
            ds.SA = instr.rs1;

            es.MA = 0x0;
            es.MB = 0x1;
            es.FS = ALUOp::ADD;

            ms.MW = false;
            ms.MD = 0x2;
            
            wbs.LD = true;
            wbs.DR = instr.rd;
            
            break;
        }
        case 0x37: {
            fs.PCJ = 0x0;
            
            ds.SA = 0x0;

            es.MA = 0x0;
            es.MB = 0x1;
            es.FS = ALUOp::ADD;

            ms.MW = false;
            ms.MD = 0x0;

            wbs.LD = true;
            wbs.DR = instr.rd;
            
            break;
        }
        case 0x17: {
            fs.PCJ = 0;
            
            es.MA = 0x1;
            es.MB = 0x1;
            es.FS = ALUOp::ADD;

            ms.MW = false;
            ms.MD = 0x0;

            wbs.LD = true;
            wbs.DR = instr.rd;
            break;
        }
        case 0x73: {
            ds.SA = 0x0;
            ds.SB = 0x0;

            es.MA = 0x0;
            es.MB = 0x0;
            es.FS = ALUOp::ADD;

            ms.MW = false;
            ms.MD = 0x0;

            wbs.LD = false;
            wbs.DR = 0x0;
            break;
        }
        default: {
            throw std::logic_error("Unknown opcode - setSigs");
        } 
    }
}

void ControlUnit::setCompareSig(CompareSig cs, uint32_t funct3, InstrType type) {
    if (type == InstrType::B_type) {
        switch(funct3) {
            case 0x0: {
                fs.PCJ = (cs.equal) ? 1 : 0;
                break;
            }
            case 0x1: {
                fs.PCJ = static_cast<uint32_t>(!cs.equal);
                break;
            }
            case 0x4: {
                fs.PCJ = static_cast<uint32_t>(cs.lessThan);
                break;
            }
            case 0x5: {
                fs.PCJ = static_cast<uint32_t>(!cs.lessThan);
                break;
            }
            case 0x6: {
                fs.PCJ = static_cast<uint32_t>(cs.lessThanU);
                break;
            }
            case 0x7: {
                fs.PCJ = static_cast<uint32_t>(!cs.lessThanU);
                break;
            }
            default: {
                throw std::logic_error("Unknown funct3");
            }
        }
    }
}

FetchSig ControlUnit::getFetchSig() {
    return fs;
}

DecodeSig ControlUnit::getDecodeSig() {
    return ds;
}

ExecuteSig ControlUnit::getExecuteSig() {
    return es;
}

MemorySig ControlUnit::getMemorySig() {
    return ms;
}

WriteBackSig ControlUnit::getWriteBackSig() {
    return wbs;
}
