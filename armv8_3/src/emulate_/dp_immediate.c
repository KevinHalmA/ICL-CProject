#include <stdint.h>
#include <assert.h>

#include "dp_immediate.h"
#include "dp_arithmetic.h"
#include "../common/utilities.h"
#include "../common/instructions.h"
#include "registers.h"

/**
 * Executes a decoded wide move instruction, using the opc to determine its type
 */ 
static void execute_dp_imm_wide_move(Instr *, CPUState *);

void decode_dp_imm(uint32_t instr, Instr *decoded) {
    // Sets fields common to every type of dp immediate instruction
    DPImmFormat format = {
        .sf  = extract_bits(instr, DP_SF_START, DP_SF_END),
        .opc = extract_bits(instr, DP_OPC_START, DP_OPC_END),
        .opi = extract_bits(instr, OPI_START, OPI_END),
        .rd  = extract_bits(instr, DP_RD_START, DP_RD_END),
    };
    
    // Determines the instruction subtype (arithmetic, wide move) using the opi
    if (format.opi == ARITHMETIC_OPI) {
        // Sets fields for arithmetic instruction
        format.imm_type = IMM_ARITHMETIC;
        format.operand.Arithmetic.sh = extract_bits(instr, SH_START, SH_END);
        format.operand.Arithmetic.imm12 = extract_bits(instr, IMM12_START, IMM12_END);
        format.operand.Arithmetic.rn = extract_bits(instr, RN_START, RN_END);
    } else {
        // Sets fields for wide move instruction
        format.imm_type = IMM_WIDE_MOVE;
        format.operand.WideMove.hw = extract_bits(instr, HW_START, HW_END);
        format.operand.WideMove.imm16 = extract_bits(instr, IMM16_START, IMM16_END);
    }

    // Sets the type and format of the decoded instruction
    decoded->type = DATA_PROCESSING_IMM;
    decoded->format.dp_imm_format = format;
}

void execute_dp_imm(Instr *instr, CPUState *cpu) {
    DPImmFormat format = instr->format.dp_imm_format;
    
    switch (format.imm_type) {
        case IMM_ARITHMETIC: ;
            uint64_t op2 = (uint64_t) format.operand.Arithmetic.imm12;
            if (format.operand.Arithmetic.sh == LEFT_SHIFT_SH) {
                op2 = logical_shift_left(op2, IMM12_LENGTH, format.sf);
            }
            DPArithmeticFormat arithmetic_format = {
                format.sf,
                format.opc,
                format.rd,
                format.operand.Arithmetic.rn,
                op2,
            };
            execute_dp_arithmetic(&arithmetic_format, cpu);
            break;
        case IMM_WIDE_MOVE:
            execute_dp_imm_wide_move(instr, cpu);
            break;
        default:
            // Assumes valid instruction - should not reach this case
            assert(0);
    };
    increment_pc(cpu);
}

static void execute_dp_imm_wide_move(Instr *instr, CPUState *cpu) {
    DPImmFormat format = instr->format.dp_imm_format;
    // Computes shift (shift = hw * 16) and operand value (op = imm16 << shift)
    uint16_t shift = format.operand.WideMove.hw * IMM16_LENGTH;
    uint64_t op = (uint64_t) format.operand.WideMove.imm16 << shift;
    // Determines the move operation using opc and sets register rd accordingly
    uint64_t value;
    switch (format.opc) {
        // Executes move wide with zero (rd := op)
        case MOVZ_OPC:
            value = op;
            break;
        // Executes move wide with NOT (rd := ~op)
        case MOVN_OPC:
            value = ~op;
            break;
        // Executes move wide with keep (rd[shift + 15 : shift] := imm16)
        case MOVK_OPC: ;
            // Reads 32-bit or 64-bit value (determined by sf) from register rd
            uint64_t reg = read_register(format.sf, cpu->registers, format.rd);
            // Creates a mask with 1s in positions 'shift' to 'shift + 15'
            uint64_t mask = get_bit_mask(shift, shift + IMM16_LENGTH - 1);
            // reg & ~mask: Clears bits shift to shift + 15 of reg to 0
            // | op:        Fills bits shift to shift + 15 of reg with imm16
            value = (reg & ~mask) | op;
            break;
        default:
            assert(0);
    };
    // Writes new value to register rd- sf = 0: 32-bit mode, sf = 1: 64-bit mode 
    write_register(format.sf, cpu->registers, format.rd, value);
}