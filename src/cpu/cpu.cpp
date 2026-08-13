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

CPU::CPU(Memory& instMem, Memory& dataMem, uint32_t entryPoint):
    instrRAM(instMem),
    dataRAM(dataMem),
    pc({entryPoint,false}),
    ishalted(false) {}


void CPU::updatePC() {
    if (pc.second && !id_ex.halt) {
        pc.first = pcMux.getOutput();
    }
}
IF_ID CPU::fetch() {
    if (!if_id.bubble) {
        updatePC();

        return {pc.first+4, pc.first,
        formatReadPC(instrRAM, pc.first), false};
    } else {
        return {pc.first,pc.first,0x13,false};
    }
    
}


ID_EX CPU::decode() {
    DecodedInstr d = decodeInstr(if_id.raw_instr);
    if (d.opcode == 0x73) {
        id_ex.halt = true;
    } else {id_ex.halt = false;}

    cu.setSigs(d);
    pcMux.setInput(0x1, add(adder, d.imm, if_id.pc_curr));
    
    regFile.setRegSigs({mem_wb.DR, cu.getDecodeSig().SA,
        cu.getDecodeSig().SB, mem_wb.LD});
    regFile.writeReg(mem_wb.Dout);
    
    setMuxInputs(FwdMuxA_ID, {regFile.getDataA(), ex_mem.aluResult, mem_wb.Dout});
    setMuxInputs(FwdMuxB_ID, {regFile.getDataB(), ex_mem.aluResult, mem_wb.Dout});

    uint32_t muxOutputA = FwdMuxA_ID.getOutput();
    uint32_t muxOutputB = FwdMuxB_ID.getOutput();

    cu.setCompareSig(
        compare(comparator, muxOutputA, muxOutputB));

    pcMux.selectInput(cu.getFetchSig().PCJ);

    return {if_id.pc_curr, if_id.pc_next, muxOutputA, muxOutputB,
        d.imm, cu.getExecuteSig(), cu.getMemorySig(), 
        cu.getWriteBackSig(), false, id_ex.halt};
}


EX_MEM CPU::execute() {

    setMuxInputs(FwdMuxA_EX, {id_ex.srcA, ex_mem.aluResult, mem_wb.Dout});
    setMuxInputs(FwdMuxB_EX, {id_ex.srcB, ex_mem.aluResult, mem_wb.Dout});

    setMuxInputs(aluInputAMux, {FwdMuxA_EX.getOutput(), id_ex.pc_curr});
    aluInputAMux.selectInput(id_ex.es.MA);
    setMuxInputs(aluInputBMux, {FwdMuxB_EX.getOutput(), id_ex.imm});
    aluInputBMux.selectInput(id_ex.es.MB);

    alu.setAluData(aluInputAMux.getOutput(), aluInputBMux.getOutput());
    alu.setAluOp(id_ex.es.FS);

    return {alu.output(), aluInputBMux.getOutput(), id_ex.pc_next, 
        id_ex.ms, id_ex.wbs, false, ex_mem.halt};
}

MEM_WB CPU::loadStoreMem() {
    ishalted = mem_wb.halt;

    dataRAM.setInputs(ex_mem.aluResult, ex_mem.data);
    dataRAM.setWriteEnable(ex_mem.ms.MW);

    formatWrite({dataRAM,ex_mem.ms.MSZ});
    uint32_t load = formatRead({dataRAM,ex_mem.ms.MSN, ex_mem.ms.MSZ});

    setMuxInputs(writeBackMux, {ex_mem.aluResult, load, ex_mem.pc_next});

    return {writeBackMux.getOutput(), ex_mem.wbs.DR, ex_mem.wbs.LD, false, mem_wb.halt};
}
