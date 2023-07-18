#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include "single_data_transfer.h"
#include "../common/utilities.h"
#include "../common/instructions.h"
#include "registers.h"
#include "memory.h"

/**
 * Returns the addressing mode of an instruction (unsigned immediate offset,
 * pre/post-index or register offset)
 * Pre: instruction is SDT (not Load Literal)
 */
static AddrMode get_addressing_mode(uint32_t);

/**
 * Executes a decoded SDT instruction
 */
static void execute_sdt(Instr *, CPUState *);

/**
 * Executes a decoded Load Literal instruction
 */
static void execute_load_literal(Instr *, CPUState *);

void decode_single_data_transfer(uint32_t instr, Instr *decoded) {
    // Sets the fields shared by SDT and Load Literal
    SDTFormat format = {
        .sf = extract_bits(instr, SF_START, SF_END),
        .rt = extract_bits(instr, RT_START, RT_END),
    };

    // Determines whether the instr is SDT or Load Literal
    uint32_t identifier = extract_bits(instr, SINGLE_DATA_TRANSFER_IDENTIFIER_START, SINGLE_DATA_TRANSFER_IDENTIFIER_END);
    if (identifier == SDT_IDENTIFIER) {
        // Sets fields for SDT type
        format.sdt_type = SDT;
        format.SDT.addr_mode = get_addressing_mode(instr);
        format.SDT.L = extract_bits(instr, L_START, L_END);
        format.SDT.xn = extract_bits(instr, XN_START, XN_END);
        // Determines the offset using the addressing mode
        switch (format.SDT.addr_mode) {
            case UNSIGNED_OFFSET:
                format.SDT.offset.imm12 = extract_bits(instr, IMM12_START, IMM12_END);
                break;
            case PRE_INDEX:
            case POST_INDEX:
                format.SDT.offset.simm9 = extract_bits(instr, SIMM9_START, SIMM9_END);
                break;
            case REGISTER_OFFSET:
                format.SDT.offset.xm = extract_bits(instr, XM_START, XM_END);
                break;
            default:
                // Assume valid instruction - should not reach this case
                assert(0);
        }
    } else {
        // Sets fields for Load Literal type
        format.sdt_type = LOAD_LITERAL;
        format.LoadLiteral.simm19 = extract_bits(instr, SIMM19_START, SIMM19_END);
    }

    // Sets the type and format of the decoded instruction
    decoded->type = SINGLE_DATA_TRANSFER;
    decoded->format.sdt_format = format;
}

static AddrMode get_addressing_mode(uint32_t instr) {
    uint32_t U_bit = extract_bits(instr, U_START, U_END);
    // If U = 1, addressing mode is Unsigned Offset
    if (U_bit == UNSIGNED_OFFSET_U) {
        return UNSIGNED_OFFSET;
    }
    uint32_t R_bit = extract_bits(instr, R_START, R_END);
    // If R = 1, addressing mode is Register Offset
    if (R_bit == REGISTER_OFFSET_R) {
        return REGISTER_OFFSET;
    }
    uint32_t I_bit = extract_bits(instr, I_START, I_END);
    // If I = 1, addressing mode is Pre-Index, otherwise Post-Index
    return I_bit == PRE_INDEX_I ? PRE_INDEX : POST_INDEX;
}

void execute_single_data_transfer(Instr *instr, CPUState *cpu) {
    if (instr->format.sdt_format.sdt_type == SDT) {
        execute_sdt(instr, cpu);
    } else {
        execute_load_literal(instr, cpu);
    }
    increment_pc(cpu);
}

static void execute_sdt(Instr *instr, CPUState *cpu) {
    SDTFormat format = instr->format.sdt_format;

    // Memory address for load/store operation
    uint64_t transfer_addr = read_register(format.sf, cpu->registers, format.SDT.xn);

    // Value to write back to Xn (only for Pre/Post-Index)
    uint64_t write_back = transfer_addr;

    // Adds correct offset (determined by addressing mode) to transfer address
    switch (format.SDT.addr_mode) {
        case UNSIGNED_OFFSET: ;
            int bit_width = format.sf == BIT_MODE_32 ? BIT_SIZE_32 : BIT_SIZE_64;
            // 32-bit mode: offset = imm12 * 4, 64-bit mode: offset = imm12 * 8
            transfer_addr += format.SDT.offset.imm12 * (bit_width / CHAR_BIT);
            break;
        case PRE_INDEX:
            transfer_addr += sign_extend(format.SDT.offset.simm9, SIMM9_LENGTH);
        case POST_INDEX:
            // For PRE/POST-INDEX, there is a write-back Xn := Xn + simm9
            write_back += sign_extend(format.SDT.offset.simm9, SIMM9_LENGTH);
            break;
        case REGISTER_OFFSET:
            transfer_addr += read_register(format.sf, cpu->registers, format.SDT.offset.xm);
            break;
        default:
            // Assume valid addressing mode - should not reach this case
            assert(0);
    }
    
    if (format.SDT.L == LOAD_L) {
        // LOAD instr type: accesses memory at transfer address, loads into rt
        uint64_t mem_val = read_memory(format.sf, cpu->memory, transfer_addr);
        write_register(format.sf, cpu->registers, format.rt, mem_val);
    } else {
        // STORE instr type: stores value of rt in memory at transfer address
        uint64_t reg_val = read_register(format.sf, cpu->registers, format.rt);
        write_memory(format.sf, cpu->memory, transfer_addr, reg_val);
    }

    // If addr mode is PRE/POST-INDEX, updates Xn using the write-back value
    if (format.SDT.addr_mode == PRE_INDEX || format.SDT.addr_mode == POST_INDEX) {
        write_register(format.sf, cpu->registers, format.SDT.xn, write_back);
    }
}

static void execute_load_literal(Instr *instr, CPUState *cpu) {
    SDTFormat format = instr->format.sdt_format;

    // Computes offset by multiplying signed 19-bit immediate value by 4
    int32_t offset = format.LoadLiteral.simm19 * INSTR_BYTES;

    // Sign extends offset to 64 bits and adds to PC to compute memory address
    uint64_t address = cpu->pc + sign_extend(offset, LOAD_LITERAL_OFFSET_LENGTH);

    // Reads value from memory address PC + simm19 * 4
    uint64_t mem_val = read_memory(format.sf, cpu->memory, address);

    // Writes value read from memory into register rt
    write_register(format.sf, cpu->registers, format.rt, mem_val);
}