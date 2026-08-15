#pragma once
#include <cstdint>

struct FwdSig_ID {
    uint32_t    idsrc;
    bool        B_type;
    
    uint32_t    memdr;
    bool        memld;
    uint32_t    memmd;

    uint32_t    wbdr;
    bool        wbld;
};

struct FwdSig_EX {
    uint32_t    exsrc;

    uint32_t    memdr;
    bool        memld;
    uint32_t    memmd;

    uint32_t    wbdr;
    bool        wbld;

};

uint32_t getInputFwdMux_ID(FwdSig_ID fwdsig);

uint32_t getInputFwdMux_EX(FwdSig_EX fwdsig);
// uint32_t getInputFwdMuxB_EX(FwdSig FwdSig);