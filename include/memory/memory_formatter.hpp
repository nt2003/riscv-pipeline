#pragma once
#include <cstdint>
#include "memory.hpp"

struct MemoryWrite {
    Memory&     memory;
    MemSize     MSZ;
};
struct MemoryRead {
    Memory&     memory;
    MemSign     MSN;
    MemSize     MSZ;
};

void formatWrite(MemoryWrite mw);

uint32_t formatRead(MemoryRead mr);

uint32_t formatReadPC(Memory& memory, uint32_t pc);