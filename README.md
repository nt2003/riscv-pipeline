# RISC-V Pipeline Simulator

A cycle-accurate RV32I simulator written in C++, modeling a real 5-stage
pipelined datapath (IF/ID/EX/MEM/WB) rather than a purely functional
instruction interpreter — built to mirror how the underlying hardware
would actually implement it.

## Features

- **Full RV32I support** — all instruction formats (R/I/S/B/U/J), decoded
  from raw 32-bit words via a hand-rolled decoder.
- **5-stage pipeline**: fetch, decode, execute, memory, writeback — with
  explicit pipeline registers (IF/ID, ID/EX, EX/MEM, MEM/WB) carrying
  control signals and data between stages.
- **Hardware-modeled ALU** — ADD/SUB/XOR/AND/OR/SLL/SRL/SRA/SLT/SLTU, all
  built on a shared adder path (ADD/SUB share one adder; SLT/SLTU derive
  their result from that adder's flags rather than a native comparison),
  with real Z/N/C/V flag computation.
- **Hazard handling** — load-use hazard detection with pipeline stalling,
  plus forwarding paths to resolve data hazards without always stalling.
- **Control-unit-driven PC redirection** — branch/jump targets resolved
  through a dedicated mux, gated so only actual branch/jump instructions
  can redirect PC.
- **Real ELF32 loading** — parses ELF headers and program headers,
  loads PT_LOAD segments at their real virtual addresses, zero-fills
  `.bss` beyond `p_filesz`.
- **Unit tested throughout** with [doctest](https://github.com/doctest/doctest) —
  memory, decoder, ALU, and full-pipeline integration tests.

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running tests

```bash
cd build
make tests
./tests
```

## Running a program

```bash
./riscv-sim path/to/program.elf
```

Programs can be assembled/linked with the RISC-V GNU toolchain
(`riscv64-unknown-elf-as`, `riscv64-unknown-elf-ld`) targeting RV32I.

## Project structure

```
include/
  cpu/          - CPU class (pipeline stages)
  datapath/     - ALU, muxes, register file
  control/      - decoder, control unit
  memory/       - Memory class, load/store formatting
  loader/       - ELF loader
src/            - implementations mirroring include/
tests/          - doctest testbenches (memory, decoder, ALU, CPU pipeline, ELF loader)
external/       - doctest.h
```

## Design notes

The datapath is built to reflect actual hardware structure rather than
just producing correct results: the ALU reuses one adder for both
ADD and SUB (subtraction computed via two's-complement negation), SLT/SLTU
are derived from that same adder's flags instead of a native `<` operator,
and LUI is implemented as `ADD rd, x0, imm` through the ALU (with a
zero-forced operand) rather than a dedicated bypass — mirroring how RISC-V's
own encoding is designed to allow exactly this reuse.

## Status

Core pipeline (fetch → decode → execute → mem/writeback) is implemented
and passing integration tests, including hazard stalling and forwarding.