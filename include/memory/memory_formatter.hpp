#pragma once
#include <cstdint>
#include "memory.hpp"
#include "memory_types.hpp"

struct MemoryWrite {
    uint32_t    addr;
    uint32_t    data;
    MemSize     MSZ;
};
struct MemoryRead {
    uint32_t    addr;
    MemSign     MSN;
    MemSize     MSZ;
};

void formatWrite(Memory& mem, MemoryWrite mw);

uint32_t formatRead(Memory& mem, MemoryRead mr);

uint32_t formatReadPC(Memory& memory, uint32_t pc);