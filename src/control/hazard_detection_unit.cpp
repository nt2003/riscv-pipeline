#include "../../include/control/hazard_detection_unit.hpp"


bool loadRegHazard(LoadHazard lh) {
    return (lh.opcode_EX == 0x3) && (lh.opcode_ID != 0x6F) && (lh.opcode_ID != 0x37) && 
    (lh.opcode_ID != 0x17) && ((lh.DR_EX == lh.SA_ID) || (lh.DR_EX == lh.SB_ID));
}

// execute opcode is load, decode opcode is not u-type or jal, execute dr is decode source A or B
