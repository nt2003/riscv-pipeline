#include "../../include/loader/elf_loader.hpp"
#include "../../include/loader/elf_types.hpp"
#include <fstream>
#include <vector>
#include <cstring>
#include <iostream>

bool loadElf(const std::string& path, Memory& instMem, Memory& dataMem, uint32_t& entryPoint) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    auto size = file.tellg();
    if (size < 0) {
        return false;
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> fileBytes(size);
    file.read(reinterpret_cast<char*>(fileBytes.data()), size);

    if (static_cast<size_t>(size) < sizeof(Elf32_Ehdr)) {
        return false;
    }
    Elf32_Ehdr header;
    std::memcpy(&header, fileBytes.data(), sizeof(Elf32_Ehdr));

    entryPoint = header.e_entry;

    instMem.setWriteEnable(true);
    dataMem.setWriteEnable(true);

    for (int i = 0; i < header.e_phnum; i++) {
        size_t offset = header.e_phoff + i * header.e_phentsize;
        if (offset + sizeof(Elf32_Phdr) > fileBytes.size()) {
            return false;
        }

        Elf32_Phdr ph;
        std::memcpy(&ph, fileBytes.data() + offset, sizeof(Elf32_Phdr));

        if (ph.p_type != PT_LOAD) {
            continue;
        }
        std::cerr << "segment " << i << ": vaddr=" << ph.p_vaddr
          << " filesz=" << ph.p_filesz
          << " flags=" << ph.p_flags << '\n';
        // Route: executable segments -> instMem, everything else -> dataMem
        Memory& target = (ph.p_flags & PF_X) ? instMem : dataMem;

        uint32_t neededSize = ph.p_vaddr + ph.p_memsz;
        if (target.size() < neededSize) {
            target.resize(neededSize);
        }

        for (uint32_t b = 0; b < ph.p_filesz; b++) {
            target.write(ph.p_vaddr + b, fileBytes.at(ph.p_offset + b));
        }
    }
    instMem.setWriteEnable(false);
    dataMem.setWriteEnable(false);
    std::cerr << "instMem[0] right after write = " << (int)instMem.read(0) << '\n';
    return true;
}