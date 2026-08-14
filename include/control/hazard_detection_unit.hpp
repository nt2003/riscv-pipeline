#pragma once

class HazardDetector {
    private:
        bool PCL = true;

    public:
        HazardDetector();

        void setPCL(bool set);

        bool getPCL();

};