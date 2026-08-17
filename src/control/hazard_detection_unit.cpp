#include "../../include/control/hazard_detection_unit.hpp"

HazardDetector::HazardDetector() : jalHazard(false), zeroRegWrite(false) {}

bool HazardDetector::loadRegHazard(LoadHazard lh) {
    return (lh.opcode_EX == 0x3) && (lh.opcode_ID != 0x6F) && (lh.opcode_ID != 0x37) && 
    (lh.opcode_ID != 0x17) && ((lh.DR_EX == lh.SA_ID) || (lh.DR_EX == lh.SB_ID));
}

bool HazardDetector::jalHazardDetect(uint32_t opcode) {
    return opcode == 0x6F;
}

void HazardDetector::setJalHazard(bool set) {
    jalHazard = set;
}

bool HazardDetector::getJalHazard() {
    return jalHazard;
}

bool HazardDetector::branchRegHazard(BranchHazard bh) {
    return (bh.opcode_ID == 0x63) && bh.LD_EX && (bh.DR_EX != 0) &&
           ((bh.DR_EX == bh.SA_ID) || (bh.DR_EX == bh.SB_ID));
}

bool HazardDetector::detectZeroRegWrite(WriteBackSig ws) {
    return (!ws.DR && ws.LD);
}

void HazardDetector::setZeroRegWrite(bool set) {
    zeroRegWrite = set;
}

bool HazardDetector::getZeroRegWrite() {
    return zeroRegWrite;
}