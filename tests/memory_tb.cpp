#include "../external/doctest.h"
#include "../include/memory/memory.hpp"
#include "../include/memory/memory_formatter.hpp"
#include <iostream>
#include <cstdint>


// ============================================================
// Memory Construction
// ============================================================

TEST_CASE("Memory - Constructor") {
    Memory mem(16);

    CHECK(mem.size() == 16);
}


TEST_CASE("Memory - Constructor Initializes To Zero") {
    Memory mem(16);

    for (uint32_t i = 0; i < 16; i++) {
        CHECK(mem.read(i) == 0);
    }
}


// ============================================================
// Raw Byte Read / Write
// ============================================================

TEST_CASE("Memory - Write Disabled") {
    Memory mem(16);

    mem.write(0, 0xAB);

    // Write should have no effect
    CHECK(mem.read(0) == 0);
}


TEST_CASE("Memory - Write Enabled") {
    Memory mem(16);

    mem.setWriteEnable(true);

    mem.write(0, 0xAB);

    CHECK(mem.read(0) == 0xAB);
}


TEST_CASE("Memory - Write Multiple Bytes") {
    Memory mem(16);

    mem.setWriteEnable(true);

    mem.write(0, 0x12);
    mem.write(1, 0x34);
    mem.write(2, 0x56);
    mem.write(3, 0x78);

    CHECK(mem.read(0) == 0x12);
    CHECK(mem.read(1) == 0x34);
    CHECK(mem.read(2) == 0x56);
    CHECK(mem.read(3) == 0x78);
}


TEST_CASE("Memory - Write Maximum Byte") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(0, 0xFF);

    CHECK(mem.read(0) == 0xFF);
}


TEST_CASE("Memory - Write Does Not Affect Other Addresses") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(1, 0xAB);

    CHECK(mem.read(0) == 0);
    CHECK(mem.read(1) == 0xAB);
    CHECK(mem.read(2) == 0);
    CHECK(mem.read(3) == 0);
}


// ============================================================
// Write Enable
// ============================================================

TEST_CASE("Memory - Disable Write After Enable") {
    Memory mem(4);

    mem.setWriteEnable(true);
    mem.write(0, 0xAA);

    mem.setWriteEnable(false);
    mem.write(0, 0xBB);

    CHECK(mem.read(0) == 0xAA);
}


TEST_CASE("Memory - Re-enable Write") {
    Memory mem(4);

    mem.setWriteEnable(true);
    mem.write(0, 0xAA);

    mem.setWriteEnable(false);
    mem.write(0, 0xBB);

    mem.setWriteEnable(true);
    mem.write(0, 0xCC);

    CHECK(mem.read(0) == 0xCC);
}


// ============================================================
// Resize
// ============================================================

TEST_CASE("Memory - Resize Larger") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(0, 0xAA);
    mem.write(3, 0xBB);

    mem.resize(8);

    CHECK(mem.size() == 8);

    CHECK(mem.read(0) == 0xAA);
    CHECK(mem.read(3) == 0xBB);

    CHECK(mem.read(4) == 0);
    CHECK(mem.read(7) == 0);
}


TEST_CASE("Memory - Resize Smaller") {
    Memory mem(8);

    mem.setWriteEnable(true);

    mem.write(0, 0xAA);
    mem.write(7, 0xBB);

    mem.resize(4);

    CHECK(mem.size() == 4);

    CHECK(mem.read(0) == 0xAA);
}


// ============================================================
// Clear
// ============================================================

TEST_CASE("Memory - Clear") {
    Memory mem(8);

    mem.setWriteEnable(true);

    mem.write(0, 0x12);
    mem.write(1, 0x34);
    mem.write(2, 0x56);
    mem.write(3, 0x78);

    mem.clear();

    CHECK(mem.size() == 0);
}


// ============================================================
// formatWrite - BYTE
// ============================================================

TEST_CASE("formatWrite - BYTE") {
    Memory mem(8);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        2,
        0xAB,
        MemSize::BYTE
    };

    formatWrite(mem, mw);

    CHECK(mem.read(2) == 0xAB);

    CHECK(mem.read(0) == 0);
    CHECK(mem.read(1) == 0);
    CHECK(mem.read(3) == 0);
}


TEST_CASE("formatWrite - BYTE Maximum") {
    Memory mem(4);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        0,
        0xFF,
        MemSize::BYTE
    };

    formatWrite(mem, mw);

    CHECK(mem.read(0) == 0xFF);
}


// ============================================================
// formatWrite - HALFWORD
// ============================================================

TEST_CASE("formatWrite - HALFWORD") {
    Memory mem(8);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        2,
        0x1234,
        MemSize::HALFWORD
    };

    formatWrite(mem, mw);

    // Little endian
    CHECK(mem.read(2) == 0x34);
    CHECK(mem.read(3) == 0x12);

    CHECK(mem.read(1) == 0);
    CHECK(mem.read(4) == 0);
}


TEST_CASE("formatWrite - HALFWORD All Bits") {
    Memory mem(4);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        0,
        0xABCD,
        MemSize::HALFWORD
    };

    formatWrite(mem, mw);

    CHECK(mem.read(0) == 0xCD);
    CHECK(mem.read(1) == 0xAB);
}


// ============================================================
// formatWrite - WORD
// ============================================================

TEST_CASE("formatWrite - WORD") {
    Memory mem(8);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        2,
        0x12345678,
        MemSize::WORD
    };

    formatWrite(mem, mw);

    // Little endian
    CHECK(mem.read(2) == 0x78);
    CHECK(mem.read(3) == 0x56);
    CHECK(mem.read(4) == 0x34);
    CHECK(mem.read(5) == 0x12);

    CHECK(mem.read(0) == 0);
    CHECK(mem.read(1) == 0);
    CHECK(mem.read(6) == 0);
    CHECK(mem.read(7) == 0);
}


TEST_CASE("formatWrite - WORD All Bits") {
    Memory mem(4);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        0,
        0xDEADBEEF,
        MemSize::WORD
    };

    formatWrite(mem, mw);

    CHECK(mem.read(0) == 0xEF);
    CHECK(mem.read(1) == 0xBE);
    CHECK(mem.read(2) == 0xAD);
    CHECK(mem.read(3) == 0xDE);
}


// ============================================================
// formatRead - BYTE
// ============================================================

TEST_CASE("formatRead - BYTE Unsigned") {
    Memory mem(4);

    mem.setWriteEnable(true);
    mem.write(0, 0xAB);

    MemoryRead mr{
        0,
        MemSign::U,
        MemSize::BYTE
    };

    CHECK(formatRead(mem,mr) == 0x000000AB);
}


TEST_CASE("formatRead - BYTE Signed Positive") {
    Memory mem(4);

    mem.setWriteEnable(true);
    mem.write(0, 0x7F);

    MemoryRead mr{
        0,
        MemSign::S,
        MemSize::BYTE
    };

    CHECK(formatRead(mem, mr) == 0x0000007F);
}


TEST_CASE("formatRead - BYTE Signed Negative") {
    Memory mem(4);

    mem.setWriteEnable(true);
    mem.write(0, 0x80);

    MemoryRead mr{
        0,
        MemSign::S,
        MemSize::BYTE
    };

    CHECK(formatRead(mem, mr) == 0xFFFFFF80);
}


TEST_CASE("formatRead - BYTE Signed -1") {
    Memory mem(4);

    mem.setWriteEnable(true);
    mem.write(0, 0xFF);

    MemoryRead mr{
        0,
        MemSign::S,
        MemSize::BYTE
    };

    CHECK(formatRead(mem, mr) == 0xFFFFFFFF);
}


TEST_CASE("formatRead - BYTE Unsigned High Value") {
    Memory mem(4);

    mem.setWriteEnable(true);
    mem.write(0, 0xFF);

    MemoryRead mr{
        0,
        MemSign::U,
        MemSize::BYTE
    };

    CHECK(formatRead(mem, mr) == 0x000000FF);
}


// ============================================================
// formatRead - HALFWORD
// ============================================================

TEST_CASE("formatRead - HALFWORD Unsigned") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(0, 0x34);
    mem.write(1, 0x12);

    MemoryRead mr{
        0,
        MemSign::U,
        MemSize::HALFWORD
    };

    CHECK(formatRead(mem, mr) == 0x00001234);
}


TEST_CASE("formatRead - HALFWORD Signed Positive") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(0, 0x34);
    mem.write(1, 0x12);

    MemoryRead mr{
        0,
        MemSign::S,
        MemSize::HALFWORD
    };

    CHECK(formatRead(mem, mr) == 0x00001234);
}


TEST_CASE("formatRead - HALFWORD Signed Negative") {
    Memory mem(4);

    mem.setWriteEnable(true);

    // 0x8000 = most negative signed 16-bit value

    mem.write(0, 0x00);
    mem.write(1, 0x80);

    MemoryRead mr{
        0,
        MemSign::S,
        MemSize::HALFWORD
    };

    CHECK(formatRead(mem, mr) == 0xFFFF8000);
}


TEST_CASE("formatRead - HALFWORD Signed -1") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(0, 0xFF);
    mem.write(1, 0xFF);

    MemoryRead mr{
        0,
        MemSign::S,
        MemSize::HALFWORD
    };

    CHECK(formatRead(mem, mr) == 0xFFFFFFFF);
}


TEST_CASE("formatRead - HALFWORD Unsigned High Value") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(0, 0xFF);
    mem.write(1, 0xFF);

    MemoryRead mr{
        0,
        MemSign::U,
        MemSize::HALFWORD
    };

    CHECK(formatRead(mem, mr) == 0x0000FFFF);
}


// ============================================================
// formatRead - WORD
// ============================================================

TEST_CASE("formatRead - WORD") {
    Memory mem(8);

    mem.setWriteEnable(true);

    mem.write(0, 0x78);
    mem.write(1, 0x56);
    mem.write(2, 0x34);
    mem.write(3, 0x12);

    MemoryRead mr{
        0,
        MemSign::U,
        MemSize::WORD
    };

    CHECK(formatRead(mem, mr) == 0x12345678);
}


TEST_CASE("formatRead - WORD All Bits") {
    Memory mem(4);

    mem.setWriteEnable(true);

    mem.write(0, 0xEF);
    mem.write(1, 0xBE);
    mem.write(2, 0xAD);
    mem.write(3, 0xDE);

    MemoryRead mr{
        0,
        MemSign::U,
        MemSize::WORD
    };

    CHECK(formatRead(mem, mr) == 0xDEADBEEF);
}


// ============================================================
// formatRead - Nonzero Address
// ============================================================

TEST_CASE("formatRead - WORD Nonzero Address") {
    Memory mem(16);

    mem.setWriteEnable(true);

    mem.write(4, 0x78);
    mem.write(5, 0x56);
    mem.write(6, 0x34);
    mem.write(7, 0x12);

    MemoryRead mr{
        4,
        MemSign::U,
        MemSize::WORD
    };

    CHECK(formatRead(mem, mr) == 0x12345678);
}


TEST_CASE("formatRead - HALFWORD Nonzero Address") {
    Memory mem(8);

    mem.setWriteEnable(true);

    mem.write(3, 0xCD);
    mem.write(4, 0xAB);

    MemoryRead mr{
        3,
        MemSign::U,
        MemSize::HALFWORD
    };

    CHECK(formatRead(mem, mr) == 0x0000ABCD);
}


// ============================================================
// formatWrite -> formatRead
// Round Trip Tests
// ============================================================

TEST_CASE("Memory - BYTE Write Read Round Trip") {
    Memory mem(8);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        3,
        0xAB,
        MemSize::BYTE
    };

    formatWrite(mem, mw);

    MemoryRead mr{
        3,
        MemSign::U,
        MemSize::BYTE
    };

    CHECK(formatRead(mem, mr) == 0xAB);
}


TEST_CASE("Memory - HALFWORD Write Read Round Trip") {
    Memory mem(8);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        2,
        0xABCD,
        MemSize::HALFWORD
    };

    formatWrite(mem, mw);

    MemoryRead mr{
        2,
        MemSign::U,
        MemSize::HALFWORD
    };

    CHECK(formatRead(mem, mr) == 0xABCD);
}


TEST_CASE("Memory - WORD Write Read Round Trip") {
    Memory mem(8);

    mem.setWriteEnable(true);

    MemoryWrite mw{
        2,
        0xDEADBEEF,
        MemSize::WORD
    };

    formatWrite(mem, mw);

    MemoryRead mr{
        2,
        MemSign::U,
        MemSize::WORD
    };

    CHECK(formatRead(mem, mr) == 0xDEADBEEF);
}


// ============================================================
// formatReadPC
// ============================================================

TEST_CASE("formatReadPC - Read Instruction") {
    Memory mem(8);

    mem.setWriteEnable(true);

    // 0x12345678
    mem.write(0, 0x78);
    mem.write(1, 0x56);
    mem.write(2, 0x34);
    mem.write(3, 0x12);

    CHECK(formatReadPC(mem, 0) == 0x12345678);
}


TEST_CASE("formatReadPC - Nonzero PC") {
    Memory mem(16);

    mem.setWriteEnable(true);

    // Instruction at address 4
    // 0xDEADBEEF

    mem.write(4, 0xEF);
    mem.write(5, 0xBE);
    mem.write(6, 0xAD);
    mem.write(7, 0xDE);

    CHECK(formatReadPC(mem, 4) == 0xDEADBEEF);
}


TEST_CASE("formatReadPC - Multiple Instructions") {
    Memory mem(16);

    mem.setWriteEnable(true);

    // Instruction 1
    mem.write(0, 0x78);
    mem.write(1, 0x56);
    mem.write(2, 0x34);
    mem.write(3, 0x12);

    // Instruction 2
    mem.write(4, 0xEF);
    mem.write(5, 0xBE);
    mem.write(6, 0xAD);
    mem.write(7, 0xDE);

    CHECK(formatReadPC(mem, 0) == 0x12345678);
    CHECK(formatReadPC(mem, 4) == 0xDEADBEEF);
}