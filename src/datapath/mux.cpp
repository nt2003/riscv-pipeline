#include "../../include/datapath/mux.hpp"

Mux::Mux(size_t size): inputs(size) {}

uint32_t Mux::getOutput() {
    if (enable) {
        return inputs.at(inputSelected);
    } else {
        return 0x0;
    }
    
}

void Mux::setInput(uint32_t slot, uint32_t data) {
    inputs.at(slot) = data;
}

void Mux::setEnable(bool ena) {
    enable = ena;
}

void Mux::selectInput(uint32_t slot) {
    inputSelected = slot;
}

uint32_t Mux::getInput(uint32_t slot) {
    return inputs.at(slot);
}