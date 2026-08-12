#pragma once
#include <cstdint>
#include "memory.hpp"

//void formatWrite(Memory& memory, MemoryData mdata, MemControl mc);

//uint32_t formatRead(Memory& memory,  MemoryData mdata, MemControl mc);

uint32_t formatReadPC(Memory& memory, uint32_t pc);