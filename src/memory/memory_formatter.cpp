#include "../../include/memory/memory_formatter.hpp"
#include <stdexcept>

void formatWrite(MemoryWrite mw) {
    switch(mw.MSZ) {
        case MemSize::BYTE: {
            mw.memory.write(mw.addr, static_cast<uint8_t>(
                mw.data));
        }
        case MemSize::HALFWORD: {
            mw.memory.write(mw.addr, static_cast<uint8_t>(
                mw.data));
            mw.memory.write(mw.addr+1, static_cast<uint8_t>(
                mw.data>>8));
        }
        case MemSize::WORD: {
            for (size_t i=0; i<4; i++) {
                mw.memory.write(mw.addr+i, static_cast<uint8_t>(
                    mw.data>>(8*i)));
            }
        }
        default: {
            throw std::logic_error("Unknown size");
        }
    }
}
uint32_t formatRead(MemoryRead mr) {
    switch(mr.MSZ) {
        case MemSize::BYTE: {
            if (mr.MSN == MemSign::S) {
                return static_cast<uint32_t>(
                    static_cast<int32_t>(
                        static_cast<int8_t>(mr.memory.read(mr.addr))));
            } else {
                return static_cast<uint32_t> (mr.memory.read(mr.addr));
            }
    
        }
        case MemSize::HALFWORD: {
            uint16_t half; 
            half += mr.memory.read(mr.addr);
            half += mr.memory.read(mr.addr+1) << 8;
            if (mr.MSN == MemSign::S) {
                return static_cast<uint32_t>(
                    static_cast<int32_t>(
                        static_cast<int16_t>(half)));
            } else {
                return static_cast<uint32_t>(half);
            }
        }
        case MemSize::WORD: {
            uint32_t word;
            for (size_t i=0; i<4; i++) {
                word = static_cast<uint32_t>(
                    mr.memory.read(mr.addr+i)) << (8*i);
            }
            return word;
        }
        default: {
            throw std::logic_error("Unknown size");
        }
    }
}
uint32_t formatReadPC(Memory& memory, uint32_t pc) {
    return formatRead({memory,pc,MemSign::U,MemSize::WORD});
}
