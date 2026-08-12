#pragma once
#include <cstdint>

struct IF_ID {
    uint32_t pc_next;
    uint32_t pc_curr;
    uint32_t raw_instr;
    bool hazard;
};