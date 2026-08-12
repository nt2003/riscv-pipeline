#include "../../include/cpu/cpu.hpp"

CPU::CPU(Memory& instMem, uint32_t entryPoint):
    instrRAM(instMem),
    pc({entryPoint,false}) {}

void CPU::updatePC() {
    if (pc.second) {
        pc.first = pcMux.getOutput();
    }
}
IF_ID CPU::fetch() {
    IF_ID nextIFID;
    updatePC();
    nextIFID.pc_curr = pc.first;
    nextIFID.pc_next = pc.first +4;
    nextIFID.raw_instr = formatReadPC(instrRAM, pc.first);
}
    
