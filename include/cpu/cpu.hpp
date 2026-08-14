#pragma once
#include <utility>
#include "../memory/memory.hpp"
#include "../memory/memory_formatter.hpp"
#include "../pipeline/pipeline_regs.hpp"
#include "../datapath/mux.hpp"
#include "../datapath/reg_file.hpp"
#include "../datapath/alu.hpp"
#include "../datapath/alu_types.hpp"
#include "../control/control_unit.hpp"
#include "../control/decoder.hpp"
#include "../control/hazard_detection_unit.hpp"


class CPU {
    private:
        //Fetch
        Memory& instrRAM;
        std::pair<uint32_t,bool> pc;
        Mux pcMux{3};
        IF_ID if_id;

        //Decode
        ControlUnit cu;
        ALU adder;
        ALU comparator;
        RegFile regFile{32};
        Mux FwdMuxA_ID{3};
        Mux FwdMuxB_ID{3};
        ID_EX id_ex;

        //Execute
        ALU alu;
        Mux FwdMuxA_EX{3};
        Mux FwdMuxB_EX{3};
        Mux aluInputAMux{2};
        Mux aluInputBMux{2};
        EX_MEM ex_mem;

        //Memory
        Memory dataRAM;
        Mux writeBackMux{3};
        MEM_WB mem_wb;

        //END PROGRAM
        bool halted;

        HazardDetector hd;

        void updatePC();

    public:
        CPU(Memory& instMem, Memory& dataMem, uint32_t entryPoint);

        IF_ID fetch();

        ID_EX decode();

        EX_MEM execute();

        MEM_WB loadStoreMem();

        void cycle();

        bool isHalted();

        std::vector<uint32_t> getRegFile();




        
        std::pair<uint32_t, bool> getPC();
        
};