#include "../include/memory/memory.hpp"
#include "../include/loader/elf_loader.hpp"
#include <string>
#include <iostream>
#include <vector>
#include "../include/cpu/cpu.hpp"
#include <stdexcept>

int main() {
    std::string path = "/Users/nicolastrejo/riscv-pipeline/mini_test.elf";
    uint32_t entryPoint;
    Memory instRAM(65536);
    Memory dataRAM(65536);

    if (!loadElf(path, instRAM, dataRAM, entryPoint)) {
        std::cerr << "Failed to load" << '\n';
        return 1;
    }

    CPU cpu(instRAM, dataRAM, entryPoint);

    try {
        int i = 0;
        while (!cpu.isHalted()) {
            cpu.cycle();
            i++;
        }
    } catch (std::runtime_error& e) {
        std::cout << "Unknown instruction " << e.what() << '\n';
        return 1;
    } catch (std::out_of_range& e) {
        std::cout << "Memory Bounds error " << e.what() << '\n';
        return 2;
    }

    std::cout << "Success!" << '\n';
    for (auto reg : cpu.getRegFile()) {
        std::cout << reg << '\n';
    } 
    return 0;
}