#pragma once
#include <cstdint>
#include "../pipeline/pipeline_regs.hpp"

// hazard_detection_unit.hpp
struct BranchHazard {
    uint32_t opcode_ID;
    uint32_t SA_ID;
    uint32_t SB_ID;
    uint32_t DR_EX;
    bool     LD_EX;
};

struct LoadHazard {
    uint32_t    opcode_ID = 0x0;
    uint32_t    SA_ID = 0x0;
    uint32_t    SB_ID = 0x0;

    uint32_t    opcode_EX = 0x0;
    uint32_t    DR_EX = 0x0;
};

class HazardDetector {
    private:
        bool jalHazard = false;

        bool zeroRegWrite = false;

    public:
        HazardDetector();

        bool loadRegHazard(LoadHazard lh);

        bool jalHazardDetect(uint32_t opcode);

        void setJalHazard(bool set);

        bool getJalHazard();

        bool branchRegHazard(BranchHazard bh);

        bool detectZeroRegWrite(WriteBackSig ws);

        void setZeroRegWrite(bool set);

        bool getZeroRegWrite();

};