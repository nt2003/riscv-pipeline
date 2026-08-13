#include "../../include/cpu/cpu.hpp"

namespace {
    uint32_t add(ALU& adder, uint32_t inputA, uint32_t inputB) {
        adder.setAluOp(ALUOp::ADD);
        adder.setAluData(inputA, inputB);
        return adder.output();
    }

    CompareSig compare(ALU& comparator, uint32_t inputA, uint32_t inputB) {
        comparator.setAluOp(ALUOp::SUB);
        comparator.setAluData(inputA, inputB);
        return comparator.getCompareSig();
    }

    void setMuxInputs(Mux& mux, const std::vector<uint32_t>& data) {
        for (size_t i=0; i<data.size(); i++) {
            mux.setInput(i, data.at(i));
        }
    }
}

CPU::CPU(Memory& instMem, uint32_t entryPoint):
    instrRAM(instMem),
    pc({entryPoint,false}) {}


void CPU::updatePC() {
    if (pc.second) {
        pc.first = pcMux.getOutput();
    }
}
IF_ID CPU::fetch() {
    updatePC();

    return {pc.first+4, pc.first,
        formatReadPC(instrRAM, pc.first), false};
}


ID_EX CPU::decode() {
    DecodedInstr d = decodeInstr(if_id.raw_instr);
    cu.setSigs(d);
    pcMux.setInput(0x1, add(adder, d.imm, if_id.pc_curr));
    
    regFile.setRegSigs({mem_wb.DR, cu.getDecodeSig().SA,
        cu.getDecodeSig().SB, mem_wb.LD});
    regFile.writeReg(mem_wb.Dout);
    
    setMuxInputs(FwdMuxA, {regFile.getDataA(), ex_mem.aluResult, mem_wb.Dout});
    setMuxInputs(FwdMuxB, {regFile.getDataB(), ex_mem.aluResult, mem_wb.Dout});

    uint32_t muxOutputA = FwdMuxA.getOutput();
    uint32_t muxOutputB = FwdMuxB.getOutput();

    cu.setCompareSig(
        compare(comparator, muxOutputA, muxOutputB));

    pcMux.selectInput(cu.getFetchSig().PCJ);

    return {if_id.pc_next, muxOutputA, muxOutputB,
        d.imm, cu.getExecuteSig(), cu.getMemorySig(), 
        cu.getWriteBackSig(), false};
}
    
