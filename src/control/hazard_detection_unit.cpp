#include "../../include/control/hazard_detection_unit.hpp"

HazardDetector::HazardDetector() {};

void HazardDetector::setPCL(bool set) {
    PCL = set;
}

bool HazardDetector::getPCL() {
    return PCL;
}