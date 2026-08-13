#include "../../include/memory/memory.hpp"

Memory::Memory(size_t size): contents(size) {}

uint8_t Memory::read(uint32_t addr) {
    return contents.at(addr);
}

void Memory::write(uint32_t addr, uint8_t data) {
    if (writeEnable) {
        contents.at(addr) = data;
    }
}

void Memory::resize(size_t size) {
    contents.resize(size);
}

void Memory::setWriteEnable(bool ena) {
    writeEnable = ena;
}

uint8_t* Memory::data() {
    return contents.data();
}

void Memory::clear() {
    contents.clear();
}

size_t Memory::size() {
    return contents.size();
}


