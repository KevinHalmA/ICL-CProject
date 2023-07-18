#include <stdlib.h>

#include "dp_arithmetic.h"
#include "../common/utilities.h"
#include "../common/instructions.h"
#include "registers.h"

/**
 * Sets condition flags in PSTATE register:
 * N - sign bit of result
 * Z = 1 if result was zero
 * C = 1 if addition produced a carry or subtraction produced a borrow
 * V = 1 if there is signed overflow/underflow
 * Pre: most recently executed instruction is arithmetic
 */
static void set_flags_arithmetic(ArithmeticType, uint8_t, uint64_t, uint64_t,
        uint64_t, CPUState *);

void execute_dp_arithmetic(DPArithmeticFormat *instr, CPUState *cpu) {
    uint8_t sf = instr->sf;
    uint8_t opc = instr->opc;
    uint8_t rd = instr->rd;
    uint8_t rn = instr->rn;
    uint64_t op2 = instr->op2;

    // Reads the value of register rn
    uint64_t op1 = read_register(sf, cpu->registers, rn);

    // Computes the result using the operation encoded by opc:
    uint64_t result;
    if (opc == ADD || opc == ADDS) {
        result = op1 + op2;
    } else {
        result = op1 - op2;
    }

    // If opc == ADDS or SUBS, set condition flags of PSTATE
    if (opc == ADDS || opc == SUBS) {
        set_flags_arithmetic(opc, sf, op1, op2, result, cpu);
    }
    
    // Writes result of arithmetic operation to register rd
    write_register(sf, cpu->registers, rd, result);
}

static void set_flags_arithmetic(ArithmeticType opc, uint8_t sf, uint64_t op1,
        uint64_t op2, uint64_t result, CPUState *cpu) {
    // Gets the position of the sign bit, depending on the bit mode
    int sign_bit = (sf == BIT_MODE_32 ? BIT_SIZE_32 : BIT_SIZE_64) - 1;

    // Gets the sign bits of op1, op2 and result
    uint64_t op1_sign = extract_bits(op1, sign_bit, sign_bit);
    uint64_t op2_sign = extract_bits(op2, sign_bit, sign_bit);
    // Truncates the result to 32 or 64 bits, depending on the bit mode
    uint64_t result_trunc = result & get_bit_mask(0, sign_bit);
    uint64_t result_sign = extract_bits(result_trunc, sign_bit, sign_bit);

    // Sets PSTATE condition flags depending on op1, op2 and result
    cpu->pstate.n_flag = result_sign;
    cpu->pstate.z_flag = result_trunc == 0;
    if (opc == ADDS) {
        // Sets C = 1 if result is less than either of the operands
        cpu->pstate.c_flag = result_trunc < op1 || result_trunc < op2;
        // Sets V = 1 if operands have same sign and result has opposite sign
        cpu->pstate.v_flag = op1_sign == op2_sign && op2_sign != result_sign;
    } else {
        // Sets C = 1 if no borrow occurred (ie: 1st operand >= 2nd operand)
        cpu->pstate.c_flag = op1 >= op2;
        // Sets V = 1 if operands have different signs, result same sign as op2
        cpu->pstate.v_flag = op1_sign != op2_sign && op2_sign == result_sign;
    }
}