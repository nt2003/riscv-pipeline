#include "../../include/control/forwarding_unit.hpp"

uint32_t getInputFwdMux_ID(FwdSig_ID fs) {
    if (fs.B_type && ((fs.idsrc == fs.memdr) || (fs.idsrc == fs.wbdr))) {
        return 0x0;
    }
    else if (fs.wbdr == fs.idsrc) {
        return 0x2;
    }
    else if ((fs.memdr == fs.idsrc) && (fs.exmd == 0)) {
        return 0x1;
    }
    else {
        return 0x0;
    }
}

uint32_t getInputFwdMux_EX(FwdSig_EX fs) {
    if (fs.exsrc == fs.wbdr) {
        return 0x2;
    } 
    else if ((fs.exsrc == fs.memdr) && (fs.memmd == 0)) {
        return 0x1;
    }
    else {
        return 0x0;
    }
}


