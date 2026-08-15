#!/usr/bin/env python3

import sys


REG_NAMES = [f"x{i}" for i in range(32)]


def sign_extend(value, bits):
    """Sign-extend value from 'bits' bits to Python int."""
    if value & (1 << (bits - 1)):
        value -= 1 << bits
    return value


def get_bits(inst, hi, lo):
    """Extract bits [hi:lo], inclusive."""
    return (inst >> lo) & ((1 << (hi - lo + 1)) - 1)


def reg(n):
    return REG_NAMES[n]


def decode(inst):
    opcode = get_bits(inst, 6, 0)
    rd = get_bits(inst, 11, 7)
    funct3 = get_bits(inst, 14, 12)
    rs1 = get_bits(inst, 19, 15)
    rs2 = get_bits(inst, 24, 20)
    funct7 = get_bits(inst, 31, 25)

    # ------------------------------------------------------------
    # R-type
    # ------------------------------------------------------------
    if opcode == 0b0110011:

        r_ops = {
            (0b000, 0b0000000): "ADD",
            (0b000, 0b0100000): "SUB",
            (0b001, 0b0000000): "SLL",
            (0b010, 0b0000000): "SLT",
            (0b011, 0b0000000): "SLTU",
            (0b100, 0b0000000): "XOR",
            (0b101, 0b0000000): "SRL",
            (0b101, 0b0100000): "SRA",
            (0b110, 0b0000000): "OR",
            (0b111, 0b0000000): "AND",
        }

        name = r_ops.get((funct3, funct7))

        if name is None:
            return "ILLEGAL"

        return f"{name} {reg(rd)} {reg(rs1)} + {reg(rs2)}"

    # ------------------------------------------------------------
    # I-type ALU
    # ------------------------------------------------------------
    if opcode == 0b0010011:

        imm = sign_extend(get_bits(inst, 31, 20), 12)

        i_ops = {
            0b000: "ADDI",
            0b010: "SLTI",
            0b011: "SLTIU",
            0b100: "XORI",
            0b110: "ORI",
            0b111: "ANDI",
        }

        # Shift immediate instructions have special encoding
        if funct3 == 0b001:
            # SLLI: funct7 must be 0000000
            if funct7 == 0b0000000:
                shamt = get_bits(inst, 24, 20)
                return f"SLLI {reg(rd)} {reg(rs1)} + {shamt}"
            return "ILLEGAL"

        if funct3 == 0b101:
            shamt = get_bits(inst, 24, 20)

            if funct7 == 0b0000000:
                return f"SRLI {reg(rd)} {reg(rs1)} + {shamt}"

            if funct7 == 0b0100000:
                return f"SRAI {reg(rd)} {reg(rs1)} + {shamt}"

            return "ILLEGAL"

        name = i_ops.get(funct3)

        if name is None:
            return "ILLEGAL"

        return f"{name} {reg(rd)} {reg(rs1)} + {imm}"

    # ------------------------------------------------------------
    # Loads
    # ------------------------------------------------------------
    if opcode == 0b0000011:

        imm = sign_extend(get_bits(inst, 31, 20), 12)

        load_ops = {
            0b000: "LB",
            0b001: "LH",
            0b010: "LW",
            0b100: "LBU",
            0b101: "LHU",
        }

        name = load_ops.get(funct3)

        if name is None:
            return "ILLEGAL"

        return f"{name} {reg(rd)} {imm}({reg(rs1)})"

    # ------------------------------------------------------------
    # Stores
    # ------------------------------------------------------------
    if opcode == 0b0100011:

        imm = (
            (get_bits(inst, 31, 25) << 5)
            | get_bits(inst, 11, 7)
        )

        imm = sign_extend(imm, 12)

        store_ops = {
            0b000: "SB",
            0b001: "SH",
            0b010: "SW",
        }

        name = store_ops.get(funct3)

        if name is None:
            return "ILLEGAL"

        return f"{name} {reg(rs2)} {imm}({reg(rs1)})"

    # ------------------------------------------------------------
    # Branches
    # ------------------------------------------------------------
    if opcode == 0b1100011:

        # B-type immediate:
        #
        # imm[12]    = inst[31]
        # imm[11]    = inst[7]
        # imm[10:5]  = inst[30:25]
        # imm[4:1]   = inst[11:8]
        # imm[0]     = 0
        #
        imm = (
            (get_bits(inst, 31, 31) << 12)
            | (get_bits(inst, 7, 7) << 11)
            | (get_bits(inst, 30, 25) << 5)
            | (get_bits(inst, 11, 8) << 1)
        )

        imm = sign_extend(imm, 13)

        branch_ops = {
            0b000: "BEQ",
            0b001: "BNE",
            0b100: "BLT",
            0b101: "BGE",
            0b110: "BLTU",
            0b111: "BGEU",
        }

        name = branch_ops.get(funct3)

        if name is None:
            return "ILLEGAL"

        return f"{name} {reg(rs1)} {reg(rs2)} + {imm}"

    # ------------------------------------------------------------
    # JAL
    # ------------------------------------------------------------
    if opcode == 0b1101111:

        # J-type immediate:
        #
        # imm[20]    = inst[31]
        # imm[10:1]  = inst[30:21]
        # imm[11]    = inst[20]
        # imm[19:12] = inst[19:12]
        # imm[0]     = 0

        imm = (
            (get_bits(inst, 31, 31) << 20)
            | (get_bits(inst, 30, 21) << 1)
            | (get_bits(inst, 20, 20) << 11)
            | (get_bits(inst, 19, 12) << 12)
        )

        imm = sign_extend(imm, 21)

        return f"JAL {reg(rd)} + {imm}"

    # ------------------------------------------------------------
    # JALR
    # ------------------------------------------------------------
    if opcode == 0b1100111:

        if funct3 != 0b000:
            return "ILLEGAL"

        imm = sign_extend(get_bits(inst, 31, 20), 12)

        return f"JALR {reg(rd)} {reg(rs1)} + {imm}"

    # ------------------------------------------------------------
    # LUI
    # ------------------------------------------------------------
    if opcode == 0b0110111:

        imm = get_bits(inst, 31, 12)

        # Display the actual upper-immediate value
        imm <<= 12

        return f"LUI {reg(rd)} + {imm}"

    # ------------------------------------------------------------
    # AUIPC
    # ------------------------------------------------------------
    if opcode == 0b0010111:

        imm = get_bits(inst, 31, 12)
        imm <<= 12

        return f"AUIPC {reg(rd)} + {imm}"

    # ------------------------------------------------------------
    # FENCE
    # ------------------------------------------------------------
    if opcode == 0b0001111:

        if funct3 == 0b000:
            return "FENCE"

        if funct3 == 0b001:
            return "FENCE.I"

        return "ILLEGAL"

    # ------------------------------------------------------------
    # SYSTEM
    # ------------------------------------------------------------
    if opcode == 0b1110011:

        # ECALL
        if funct3 == 0b000 and get_bits(inst, 31, 20) == 0:
            if rs1 == 0 and rd == 0:
                return "ECALL"
            return "ILLEGAL"

        # EBREAK
        if funct3 == 0b000 and get_bits(inst, 31, 20) == 1:
            if rs1 == 0 and rd == 0:
                return "EBREAK"
            return "ILLEGAL"

        # CSR instructions
        csr = get_bits(inst, 31, 20)

        csr_ops = {
            0b001: "CSRRW",
            0b010: "CSRRS",
            0b011: "CSRRC",
            0b101: "CSRRWI",
            0b110: "CSRRSI",
            0b111: "CSRRCI",
        }

        name = csr_ops.get(funct3)

        if name is None:
            return "ILLEGAL"

        # Register versions
        if funct3 in (0b001, 0b010, 0b011):
            return f"{name} {reg(rd)} {hex(csr)} {reg(rs1)}"

        # Immediate versions
        zimm = rs1
        return f"{name} {reg(rd)} {hex(csr)} {zimm}"

    # ------------------------------------------------------------
    # Unknown opcode
    # ------------------------------------------------------------
    return "ILLEGAL"


def parse_instruction(text):
    text = text.strip().replace("_", "")

    # Binary
    if text.startswith("0b"):
        return int(text, 2)

    # Hex
    if text.startswith("0x"):
        return int(text, 16)

    # If it consists only of 0/1 and has 32 bits, treat as binary
    if len(text) == 32 and all(c in "01" for c in text):
        return int(text, 2)

    # Otherwise assume hexadecimal
    return int(text, 16)


def main():
    if len(sys.argv) > 1:
        inputs = sys.argv[1:]
    else:
        # Interactive mode
        print("RV32I decoder")
        print("Enter a hex or 32-bit binary instruction.")
        print("Type 'q' to quit.\n")

        while True:
            text = input("> ").strip()

            if text.lower() in ("q", "quit", "exit"):
                break

            try:
                inst = parse_instruction(text)

                if inst < 0 or inst > 0xFFFFFFFF:
                    print("Error: instruction must be 32 bits.")
                    continue

                print(f"{inst:08x}  ->  {decode(inst)}")

            except ValueError:
                print("Error: invalid instruction.")
        return

    # Command-line mode
    for text in inputs:
        try:
            inst = parse_instruction(text)

            if inst < 0 or inst > 0xFFFFFFFF:
                print(f"{text}: instruction must be 32 bits.")
                continue

            print(f"{inst:08x}  ->  {decode(inst)}")

        except ValueError:
            print(f"{text}: invalid instruction")

main()
