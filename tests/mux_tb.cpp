#include "../external/doctest.h"
#include "../include/datapath/mux.hpp"
#include <iostream>

TEST_CASE("Mux - Constructor") {
    Mux mux(4);

    // Default selected input is 0
    // Default enable is true
    CHECK(mux.getOutput() == 0);
}


TEST_CASE("Mux - Set and Select Input") {
    Mux mux(4);

    mux.setInput(0, 10);
    mux.setInput(1, 20);
    mux.setInput(2, 30);
    mux.setInput(3, 40);

    // Select input 0
    mux.selectInput(0);
    CHECK(mux.getOutput() == 10);

    // Select input 1
    mux.selectInput(1);
    CHECK(mux.getOutput() == 20);

    // Select input 2
    mux.selectInput(2);
    CHECK(mux.getOutput() == 30);

    // Select input 3
    mux.selectInput(3);
    CHECK(mux.getOutput() == 40);
}


TEST_CASE("Mux - Default Selection") {
    Mux mux(4);

    mux.setInput(0, 0x12345678);
    mux.setInput(1, 0xAAAAAAAA);
    mux.setInput(2, 0x55555555);
    mux.setInput(3, 0xFFFFFFFF);

    // inputSelected defaults to 0
    CHECK(mux.getOutput() == 0x12345678);
}


TEST_CASE("Mux - Change Selection") {
    Mux mux(4);

    mux.setInput(0, 100);
    mux.setInput(1, 200);

    mux.selectInput(0);
    CHECK(mux.getOutput() == 100);

    mux.selectInput(1);
    CHECK(mux.getOutput() == 200);

    mux.selectInput(0);
    CHECK(mux.getOutput() == 100);
}


TEST_CASE("Mux - Enable") {
    Mux mux(4);

    mux.setInput(0, 0x12345678);
    mux.selectInput(0);

    // Enabled by default
    CHECK(mux.getOutput() == 0x12345678);

    // Disable mux
    mux.setEnable(false);

    // Disabled output should be 0
    CHECK(mux.getOutput() == 0);

    // Re-enable mux
    mux.setEnable(true);

    CHECK(mux.getOutput() == 0x12345678);
}


TEST_CASE("Mux - Enable and Selection") {
    Mux mux(4);

    mux.setInput(0, 100);
    mux.setInput(1, 200);

    mux.selectInput(1);

    // Enabled
    mux.setEnable(true);
    CHECK(mux.getOutput() == 200);

    // Disabled
    mux.setEnable(false);
    CHECK(mux.getOutput() == 0);

    // Change selection while disabled
    mux.selectInput(0);
    CHECK(mux.getOutput() == 0);

    // Re-enable -> new selection should be active
    mux.setEnable(true);
    CHECK(mux.getOutput() == 100);
}


TEST_CASE("Mux - Input Values") {
    Mux mux(4);

    // Test important uint32_t boundary values
    mux.setInput(0, 0x00000000);
    mux.setInput(1, 0xFFFFFFFF);
    mux.setInput(2, 0x80000000);
    mux.setInput(3, 0x7FFFFFFF);

    mux.selectInput(0);
    CHECK(mux.getOutput() == 0x00000000);

    mux.selectInput(1);
    CHECK(mux.getOutput() == 0xFFFFFFFF);

    mux.selectInput(2);
    CHECK(mux.getOutput() == 0x80000000);

    mux.selectInput(3);
    CHECK(mux.getOutput() == 0x7FFFFFFF);
}


TEST_CASE("Mux - Overwrite Input") {
    Mux mux(4);

    mux.setInput(0, 100);
    mux.selectInput(0);

    CHECK(mux.getOutput() == 100);

    // Overwrite existing input
    mux.setInput(0, 200);

    CHECK(mux.getOutput() == 200);
}


TEST_CASE("Mux - Overwrite Multiple Inputs") {
    Mux mux(4);

    mux.setInput(0, 10);
    mux.setInput(1, 20);
    mux.setInput(2, 30);
    mux.setInput(3, 40);

    mux.selectInput(1);
    CHECK(mux.getOutput() == 20);

    // Change input 1
    mux.setInput(1, 999);
    CHECK(mux.getOutput() == 999);

    // Other inputs should remain unchanged
    mux.selectInput(0);
    CHECK(mux.getOutput() == 10);

    mux.selectInput(2);
    CHECK(mux.getOutput() == 30);

    mux.selectInput(3);
    CHECK(mux.getOutput() == 40);
}


TEST_CASE("Mux - Disable Then Re-enable") {
    Mux mux(2);

    mux.setInput(0, 111);
    mux.setInput(1, 222);

    mux.selectInput(1);

    CHECK(mux.getOutput() == 222);

    mux.setEnable(false);
    CHECK(mux.getOutput() == 0);

    mux.setEnable(true);
    CHECK(mux.getOutput() == 222);

    mux.setEnable(false);
    CHECK(mux.getOutput() == 0);

    mux.setEnable(true);
    CHECK(mux.getOutput() == 222);
}


TEST_CASE("Mux - Single Input") {
    Mux mux(1);

    mux.setInput(0, 0xDEADBEEF);

    // Only valid input is 0
    mux.selectInput(0);

    CHECK(mux.getOutput() == 0xDEADBEEF);

    mux.setEnable(false);
    CHECK(mux.getOutput() == 0);

    mux.setEnable(true);
    CHECK(mux.getOutput() == 0xDEADBEEF);
}


TEST_CASE("Mux - Many Inputs") {
    Mux mux(16);

    // Give every input a unique value
    for (uint32_t i = 0; i < 16; i++) {
        mux.setInput(i, i * 100);
    }

    // Verify every input can be selected
    for (uint32_t i = 0; i < 16; i++) {
        mux.selectInput(i);
        CHECK(mux.getOutput() == i * 100);
    }
}


TEST_CASE("Mux - Disabled Output Independent of Input") {
    Mux mux(4);

    mux.setInput(0, 0x11111111);
    mux.setInput(1, 0x22222222);
    mux.setInput(2, 0x33333333);
    mux.setInput(3, 0x44444444);

    mux.setEnable(false);

    mux.selectInput(0);
    CHECK(mux.getOutput() == 0);

    mux.selectInput(1);
    CHECK(mux.getOutput() == 0);

    mux.selectInput(2);
    CHECK(mux.getOutput() == 0);

    mux.selectInput(3);
    CHECK(mux.getOutput() == 0);
}