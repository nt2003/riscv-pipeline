#include "../../include/datapath/mux.hpp"

Mux::Mux(size_t size): inputs(size) {}

uint32_t Mux::getOutput() {
    return inputs.at(inputSelected);
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