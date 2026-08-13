#pragma once
#include <vector>

class Memory {
    private:
       std::vector<uint32_t> files;

    public:
        Memory(size_t size);
};