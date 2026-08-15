#pragma once
#include <vector>
#include "memory_types.hpp"

class Memory {
    private:
       std::vector<uint8_t> contents;

       bool writeEnable = false;

    public:
        Memory(size_t size);

        uint8_t read(uint32_t addr);

        void write(uint32_t addr, uint8_t data);

        void resize(size_t size);

        void setWriteEnable(bool ena);

        uint8_t* data();

        void clear();

        size_t size();



        std::vector<uint8_t> getContents();
};