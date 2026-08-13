#pragma once
#include <cstdint>
#include "memory.hpp"
#include "memory_types.hpp"

struct MemoryWrite {
    Memory&     memory;
    uint32_t    addr;
    uint32_t    data;
    MemSize     MSZ;
};
struct MemoryRead {
    Memory&     memory;
    uint32_t    addr;
    MemSign     MSN;
    MemSize     MSZ;
};

void formatWrite(MemoryWrite mw);

uint32_t formatRead(MemoryRead mr);

uint32_t formatReadPC(Memory& memory, uint32_t pc);