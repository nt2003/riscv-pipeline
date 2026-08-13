#pragma once
#include <string>
#include "../memory/memory.hpp"
#include "../memory/memory_formatter.hpp"

/**
 * @brief Loads an ELF executable into memory, copying each PT_LOAD segment
 *        to its linked virtual address.
 * @param path Path to the ELF file on disk.
 * @param memory Memory object to load segments into.
 * @param entryPoint Filled in with the ELF's entry point address on success.
 * @return true on success, false if the file couldn't be opened or isn't valid ELF32.
 */
bool loadElf(const std::string& path, Memory& instMem, Memory& dataMem, uint32_t& entryPoint);