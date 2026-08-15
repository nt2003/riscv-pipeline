#include "../../include/cpu/cpu.hpp"
#include <iostream>
#include <bitset>
#include <iomanip>

bool test = false;

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

    DecodedInstr NOP() {
        return decodeInstr(0x13);
    }
}

CPU::CPU(Memory& instMem, Memory& dataMem, uint32_t entryPoint):
    instrRAM(instMem),
    dataRAM(dataMem),
    pc({entryPoint,false}),
    halted(false), 
    if_id{0, 0, 0, true, false}  // bubble = true: nothing fetched yet before cycle 1
        {pcMux.setInput(0x0, entryPoint);
        pcMux.selectInput(0x0);}


void CPU::updatePC() {
    if (!pc.second || id_ex.halt) {
        return; // PCL held low (hazard stall), or halted — hold PC
    }
    pc.first = pcMux.getOutput();
}
std::pair<uint32_t, bool> CPU::getPC() {
    return pc;
}

IF_ID CPU::fetch() {
    updatePC();

    if (test) {
        std::cout << +pc.first << '\n';
        test = false;
    }

    pc.second = !if_id.stall;

    uint32_t currentPC = pc.first;
    uint32_t nextpc = pc.first + 4;
    uint32_t rawInstr = formatRead(instrRAM, {currentPC, MemSign::S, MemSize::WORD});

    pcMux.setInput(0x0, nextpc);

    return IF_ID{nextpc, currentPC, rawInstr, false, false};
}


ID_EX CPU::decode() {
    bool bubble = false;
    if (if_id.bubble) {
        ID_EX bubble;
        bubble.bubble = true;
        return bubble; // everything else stays at struct defaults (all zero/false)
    }

    DecodedInstr d = decodeInstr(if_id.raw_instr);
    bool halt = (d.opcode == 0x73);

    if (loadRegHazard({d.opcode, d.rs1, d.rs2, id_ex.opcode, id_ex.wbs.DR})) {
        pc.second = false;

        ID_EX bubble{};
        bubble.bubble = true;
        return bubble;
    }
    

    pcMux.setInput(0x1, add(adder, d.imm, if_id.pc_curr));
    cu.setSigs(d);

    regFile.setRegSigs({mem_wb.DR, cu.getDecodeSig().SA,
        cu.getDecodeSig().SB, mem_wb.LD});
    regFile.writeReg(mem_wb.Dout);
    
    setMuxInputs(FwdMuxA_ID, {regFile.getDataA(), ex_mem.aluResult, mem_wb.Dout});
    setMuxInputs(FwdMuxB_ID, {regFile.getDataB(), ex_mem.aluResult, mem_wb.Dout});

    FwdMuxA_ID.selectInput(
                    getInputFwdMux_ID(
                        {cu.getDecodeSig().SA,
                        d.type == InstrType::B_type,     
                        ex_mem.wbs.DR,
                        ex_mem.wbs.LD,
                        ex_mem.ms.MD,
                        mem_wb.DR,
                        mem_wb.LD}));

    FwdMuxB_ID.selectInput(
                    getInputFwdMux_ID(
                        {cu.getDecodeSig().SB,
                        d.type == InstrType::B_type,     
                        ex_mem.wbs.DR,
                        ex_mem.wbs.LD,
                        ex_mem.ms.MD,
                        mem_wb.DR,
                        mem_wb.LD}));


    uint32_t muxOutputA = FwdMuxA_ID.getOutput();
    uint32_t muxOutputB = FwdMuxB_ID.getOutput();

    cu.setCompareSig(
        compare(comparator, muxOutputA, muxOutputB), d.funct3, d.type);

    pcMux.selectInput(cu.getFetchSig().PCJ);

    return {if_id.pc_curr, if_id.pc_next, muxOutputA, muxOutputB,
        d.imm, cu.getDecodeSig().SA, cu.getDecodeSig().SB, cu.getExecuteSig(), 
        cu.getMemorySig(), cu.getWriteBackSig(), bubble, halt, d.opcode};
}


EX_MEM CPU::execute() {

    setMuxInputs(FwdMuxA_EX, {id_ex.srcA, ex_mem.aluResult, mem_wb.Dout});
    setMuxInputs(FwdMuxB_EX, {id_ex.srcB, ex_mem.aluResult, mem_wb.Dout});
   
    FwdMuxA_EX.selectInput(
                    getInputFwdMux_EX(
                        {id_ex.EX_SA,
                        ex_mem.wbs.DR,
                        ex_mem.wbs.LD,
                        ex_mem.ms.MD,
                        mem_wb.DR,
                        mem_wb.LD}
                    ));
    FwdMuxB_EX.selectInput(
                    getInputFwdMux_EX(
                        {id_ex.EX_SB,
                        ex_mem.wbs.DR,
                        ex_mem.wbs.LD,
                        ex_mem.ms.MD,
                        mem_wb.DR,
                        mem_wb.LD}
                    ));

    setMuxInputs(aluInputAMux, {FwdMuxA_EX.getOutput(), id_ex.pc_curr});
    aluInputAMux.selectInput(id_ex.es.MA);
    setMuxInputs(aluInputBMux, {FwdMuxB_EX.getOutput(), id_ex.imm});
    aluInputBMux.selectInput(id_ex.es.MB);

    alu.setAluData(aluInputAMux.getOutput(), aluInputBMux.getOutput());
    alu.setAluOp(id_ex.es.FS);

    pcMux.setInput(0x2, alu.output());
    return {alu.output(), FwdMuxB_EX.getOutput(), id_ex.pc_next, 
        id_ex.ms, id_ex.wbs, id_ex.halt};
}

MEM_WB CPU::loadStoreMem() {
    halted = ex_mem.halt;

    dataRAM.setWriteEnable(ex_mem.ms.MW);

    formatWrite(dataRAM, {ex_mem.aluResult, ex_mem.data, ex_mem.ms.MSZ});

    uint32_t load = (ex_mem.ms.MD == 1) ? formatRead(dataRAM, {ex_mem.aluResult ,ex_mem.ms.MSN, 
        ex_mem.ms.MSZ}) : 0;

    setMuxInputs(writeBackMux, {ex_mem.aluResult, load, ex_mem.pc_next});
    std::cout << ex_mem.pc_next << '\n';
    writeBackMux.selectInput(ex_mem.ms.MD);
    std::cout << +ex_mem.ms.MD << ", " << +writeBackMux.getOutput() << '\n';
    return {writeBackMux.getOutput(), ex_mem.wbs.DR, ex_mem.wbs.LD};
}

void CPU::cycle() {
    IF_ID ifid_next = fetch();
    ID_EX idex_next = decode();

    // Hazard detected during decode:
    // keep the dependent instruction in IF/ID.
    if (!pc.second) {
        ifid_next = if_id;
    }

    EX_MEM exmem_next = execute();
    MEM_WB memwb_next = loadStoreMem();

    if_id = ifid_next;
    id_ex = idex_next;
    ex_mem = exmem_next;
    mem_wb = memwb_next;
}

bool CPU::isHalted() {
    return halted;
}

std::vector<uint32_t> CPU::getRegFile() {
    return regFile.getRegFile();
}