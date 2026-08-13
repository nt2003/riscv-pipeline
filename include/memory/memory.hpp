#pragma once
#include <vector>

class Memory {
    private:
       std::vector<uint32_t> contents;

       uint32_t addr;

       uint32_t data;

    public:
        Memory(size_t size);

        void setInputs(uint32_t addr, uint32_t data);

        void setWriteEnable(bool ena);

        void write(uint32_t addr, uint8_t byte);
};