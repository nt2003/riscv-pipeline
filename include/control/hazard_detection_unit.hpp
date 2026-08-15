#pragma once
#include <cstdint>

struct LoadHazard {
    uint32_t    opcode_ID;
    uint32_t    SA_ID;
    uint32_t    SB_ID;

    uint32_t    opcode_EX;
    uint32_t    DR_EX;
};

bool loadRegHazard(LoadHazard lh);

bool jalHazard (uint32_t opcode);
