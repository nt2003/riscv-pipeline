#include "../../include/control/hazard_detection_unit.hpp"


bool loadRegHazard(LoadHazard lh) {
    return (lh.opcode_EX == 0x3) && (lh.opcode_ID != 0x6F) && (lh.opcode_ID != 0x37) && 
    (lh.opcode_ID != 0x17) && ((lh.DR_EX == lh.SA_ID) || (lh.DR_EX == lh.SB_ID));
}

bool jalHazard(uint32_t opcode) {
    return opcode == 0x6F;
}