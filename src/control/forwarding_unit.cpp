#include "../../include/control/forwarding_unit.hpp"

uint32_t getInputFwdMux_ID(FwdSig_ID fs) {
    if (fs.B_type) {
        return 0x0;
    }
    else if (fs.memld && (fs.memdr == fs.idsrc) && (fs.memmd == 0) ) {
        return 0x1;
    }
    else if (fs.wbld && (fs.wbdr == fs.idsrc)) {
        return 0x2;
    }
    else {
        return 0x0;
    }
}

uint32_t getInputFwdMux_EX(FwdSig_EX fs) {
    if (fs.memld && (fs.memdr == fs.exsrc) && (fs.memmd == 0)) {
        return 0x1;
    }
    else if (fs.wbld && (fs.wbdr == fs.exsrc)) {
        return 0x2;
    } 
    else {
        return 0x0;
    }
}


