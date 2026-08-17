#pragma once
#include <cstdint>

struct RegfileControl_ID {

    uint32_t    SA;
    uint32_t    SB;
    
};

struct RegfileControl_WB {
    uint32_t    DR;
    bool        LD;
};