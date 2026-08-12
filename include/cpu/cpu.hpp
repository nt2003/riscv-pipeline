#pragma once
#include <utility>
#include "../memory/memory.hpp"
#include "../memory/memory_formatter.hpp"
#include "../pipeline/pipeline_regs.hpp"
#include "../datapath/mux.hpp"

class CPU {
    private:
        //Fetch
        Memory& instrRAM;
        std::pair<uint32_t,bool> pc;
        Mux pcMux{3};
        IF_ID if_id;



    public:
        CPU(Memory& instMem, uint32_t entryPoint);

        IF_ID fetch();
        void updatePC();
};