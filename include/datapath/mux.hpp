#pragma once
#include <vector>

class Mux {
    private:
        std::vector<uint32_t> inputs;

        uint32_t inputSelected;

        bool enable;

    public:
        Mux(size_t size);

        uint32_t getOutput();

        void setInput(uint32_t input);

        void setEnable(bool ena);
};
