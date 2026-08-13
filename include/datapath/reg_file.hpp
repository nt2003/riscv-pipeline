#pragma once
#include <vector>
#include "../control/regfile_controls.hpp"

class RegFile {
    private:
        std::vector<uint32_t> contents;

        //LD control bit, write enable for regfile
        bool writeEnable = false;

        uint32_t srcAddrA = 0x0;

        uint32_t srcAddrB = 0x0;

        uint32_t destReg = 0x0;

    public:
        RegFile(size_t size);

        void setRegSigs(RegfileControl rc);

        void writeReg(uint32_t dataIn);


        uint32_t getDataA();

        uint32_t getDataB();

        //void resetBits();

};