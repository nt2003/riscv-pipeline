#include "../include/memory/memory.hpp"
#include "../include/loader/elf_loader.hpp"
#include <string>
#include <iostream>
#include <vector>
#include "../include/cpu/cpu.hpp"
#include <stdexcept>

int main() {
    std::string path = "/Users/nicolastrejo/riscv-sim/tests/mini.elf";
    uint32_t entryPoint;
    Memory instRAM(65536);
    Memory dataRAM(65536);

    if (!loadElf(path, instRAM, dataRAM, entryPoint)) {
        std::cerr << "Failed to load" << '\n';
        return 1;
    }
    std::cout << entryPoint << '\n';
    std::cout << formatReadPC(instRAM, entryPoint) << '\n';

    CPU cpu(instRAM, dataRAM, entryPoint);

    try {
        while (!cpu.isHalted()) {
            cpu.cycle();
        }
    } catch (std::runtime_error& e) {
        std::cout << "Unknown instruction " << e.what() << '\n';
    } catch (std::out_of_range& e) {
        std::cout << "Memory Bounds error " << e.what() << '\n';
    }

    std::cout << "Success!" << '\n';
    for (auto reg : cpu.getRegFile()) {
        std::cout << reg << '\n';
    } 
    return 0;
}