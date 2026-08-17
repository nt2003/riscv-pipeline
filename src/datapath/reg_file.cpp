#include "../../include/datapath/reg_file.hpp"

RegFile::RegFile(size_t size): contents(size) {}

void RegFile::setRegSigs_ID(RegfileControl_ID rc) {
    srcAddrA = rc.SA;
    srcAddrB = rc.SB;
}

void RegFile::setRegSigs_WB(RegfileControl_WB rc) {
    destReg = rc.DR;
    writeEnable = rc.LD;
}

void RegFile::writeReg(uint32_t dataIn) {
    if (writeEnable && (destReg != 0x0)) {
        contents.at(destReg) = dataIn;
    }
}

uint32_t RegFile::getDataA() {
    return contents.at(srcAddrA);
}

uint32_t RegFile::getDataB() {
    return contents.at(srcAddrB);
}

std::vector<uint32_t> RegFile::getRegFile() {
    return contents;
}




bool RegFile::getEnable() {
    return writeEnable;
}

uint32_t RegFile::getDestReg() {
    return destReg;
}