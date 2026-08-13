#pragma once
#include <vector>
#include "../control/regfile_controls.hpp"

class RegFile {
    private:
        std::vector<uint32_t> file;

        //LD control bit, write enable for regfile
        bool writeEnable;

        uint32_t srcAddrA;

        uint32_t srcAddrB;

        uint32_t destReg;

    public:
        RegFile(size_t size);

        void setRegSigs(RegfileControl rc);

        void writeReg(uint32_t dataIn);


        uint32_t getDataA();

        uint32_t getDataB();

        void resetBits();

};