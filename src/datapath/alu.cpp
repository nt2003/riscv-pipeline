#include "../../include/datapath/alu.hpp"
#include <cstddef>

ALU::ALU() : Z_flag(false), N_flag(false), V_flag(false), C_flag(false) {};

void ALU::setAluData(uint32_t inputA, uint32_t inputB) {
    inputA = inputA;
    inputB = inputB;
}

void ALU::setAluOp(ALUOp op) {
    op = op;
}

namespace {
    uint8_t msb(uint32_t input) {
        return input>>31;
    }
}

void ALU::resetFlags() {
        Z_flag = false;
        N_flag = false;
        V_flag = false;
        C_flag = false;
    }

uint32_t ALU::output() {
    resetFlags();
    uint32_t ans;
    switch(op) {
        case ALUOp::ADD:{ 
            ans = inputA + inputB;
            uint64_t fullSum = static_cast<uint64_t>(inputA) + static_cast<uint64_t>(inputB);
            if (ans == 0) {Z_flag = true;}
            if ((ans >>31) == 1) {N_flag = true;}
            if (ans < fullSum) {C_flag = true;}
            if ((msb(inputA) == msb(inputB)) && (msb(inputA) != msb(ans))) {V_flag = true;}
            break;
        }
        case ALUOp::SUB:{
            uint32_t negB = ~inputB + 1;
            ans = inputA + negB;
            uint64_t fullSum = static_cast<uint64_t>(inputA) + static_cast<uint64_t>(negB);
            if (ans == 0) {Z_flag = true;}
            if ((ans >>31) == 1) {N_flag = true;}
            if (ans < fullSum) {C_flag = true;}
            if ((msb(inputA) == msb(inputB)) && (msb(inputA) != msb(ans))) {V_flag = true;}
            break;
        }
        case ALUOp::XOR:{
            ans = inputA ^ inputB;
            if (ans == 0) {Z_flag = true;}
            if ((ans>>31) == 1) {N_flag = true;}
            break;
        }
        case ALUOp::AND:{
            ans = inputA & inputB;
            if (ans == 0) {Z_flag = true;}
            if ((ans>>31) == 1) {N_flag = true;}
            break;
        }
        case ALUOp::OR:{
            ans = inputA | inputB;
            if (ans == 0) {Z_flag = true;}
            if ((ans>>31) == 1) {N_flag = true;}
            break;
        }
        case ALUOp::SLL:{
            ans = inputA << inputB;
            if (ans == 0) {Z_flag = true;}
            if ((ans>>31) == 1) {N_flag = true;}
            break;
        }
        case ALUOp::SRL:{
            ans = inputA >> inputB;
            if (ans == 0) {Z_flag = true;}
            if ((ans>>31) == 1) {N_flag = true;}
            break;
        }
        case ALUOp::SRA:{
            ans = static_cast<uint32_t>(static_cast<int32_t>(inputA) >> inputB);
            if (ans == 0) {Z_flag = true;}
            if ((ans>>31) == 1) {N_flag = true;}
            break;
        }
        case ALUOp::SLT:{
            uint32_t negB = ~inputB + 1;
            ans = inputA + negB;
            uint64_t fullSum = static_cast<uint64_t>(inputA) + static_cast<uint64_t>(negB);
            if (ans == 0) {Z_flag = true;}
            if ((ans >>31) == 1) {N_flag = true;}
            if (ans < fullSum) {C_flag = true;}
            if ((msb(inputA) == msb(inputB)) && (msb(inputA) != msb(ans))) {V_flag = true;}
            
            ans = N_flag ^ V_flag;
            break;
        }
        case ALUOp::SLTU:{
            uint32_t negB = ~inputB + 1;
            ans = inputA + negB;
            uint64_t fullSum = static_cast<uint64_t>(inputA) + static_cast<uint64_t>(negB);
            if (ans == 0) {Z_flag = true;}
            if ((ans >>31) == 1) {N_flag = true;}
            if (ans < fullSum) {C_flag = true;}
            if ((msb(inputA) == msb(inputB)) && (msb(inputA) != msb(ans))) {V_flag = true;}
            
            ans = !C_flag;
            break;
        }
    }

    return ans;
}


CompareSig ALU::getCompareSig() {
   output();

   return {Z_flag, (N_flag != V_flag), !C_flag};
}

ALUFlags ALU::getFlags() {
    return{Z_flag, N_flag, C_flag, V_flag};
}

bool ALU::getZ() {
    return Z_flag;
}
bool ALU::getN() {
    return N_flag;
}
bool ALU::getC() {
    return C_flag;
}
bool ALU::getV() {
    return V_flag;
}