#pragma once
#include <cstdint>

struct RegfileControl {
    uint32_t    DR;
    uint32_t    SA;
    uint32_t    SB;
    bool        LD;
};