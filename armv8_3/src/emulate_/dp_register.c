#include <stdint.h>
#include <assert.h>

#include "dp_register.h"
#include "dp_arithmetic.h"
#include "../common/utilities.h"
#include "../common/instructions.h"
#include "registers.h"

/**
 * Declares a type ShiftPtr representing a pointer to a shift function
 */
typedef uint64_t (*ShiftPtr)(uint64_t, int, int);

/**
 * Defines an array of pointers to shift functions
 */
static ShiftPtr shiftTable[] = {
    &logical_shift_left,
    &logical_shift_right,
    &arithmetic_shift_right,
    &rotate_right,
};

/**
 * Defines a macro for defining binary operations
 */
#define OPERATOR_FUNCTION(name, operator)\
static uint64_t name(uint64_t a, uint64_t b)\
{\
return a operator b;\
}

/**
 * Defines 3 bitwise binary operations
 */
OPERATOR_FUNCTION(and, &)
OPERATOR_FUNCTION(or, |)
OPERATOR_FUNCTION(xor, ^)

/**
 * Declares a type LogicPtr representing a pointer to a bitwise logical function
 */
typedef uint64_t (*LogicPtr)(uint64_t, uint64_t);

/**
 * Defines an array of pointers to bitwise logical functions
 */
static LogicPtr logicTable[] = {
    &and,
    &or,
    &xor,
    &and,
};

/**
 * Executes a decoded multiply instruction
 */
static void execute_dp_reg_multiply(Instr *, CPUState *);

/**
 * Executes a decoded arithmetic instruction
 */
static void execute_dp_reg_arithmetic(Instr *, CPUState *);

/** 
 * Executes a decoded logical instruction encoded by a combination of opc and N:
 * opc N  Instruction
 * 00  0  and
 * 00  1  bic
 * 01  0  orr
 * 01  1  orn
 * 10  0  eor
 * 10  1  eon
 * 11  0  ands
 * 11  1  bics
 */
static void execute_dp_reg_logical(Instr *, CPUState *);

/**
 * Returns op2 - the value of register Rm shifted by operand many bits, using
 * the shift operation encoded by shift:
 * shift operation
 * 00    lsl
 * 01    lsr
 * 10    asr
 * 11    ror (logical instructions only)
 */
static uint64_t arithmetic_logical_shift(Instr *, CPUState *);

/**
 * Sets condition flags in PSTATE register:
 * N - sign bit of result
 * Z - 1 if result was zero
 * C - 0
 * V - 0
 * Pre: most recently executed instruction is logical
 */
static void set_flags_logical(uint8_t, uint64_t, CPUState *);

void decode_dp_reg(uint32_t instr, Instr *decoded) {
    // Sets fields common to every type of dp register instruction
    DPRegFormat format = {
        .sf  = extract_bits(instr, DP_SF_START, DP_SF_END),
        .opc = extract_bits(instr, DP_OPC_START, DP_OPC_END),
        .M   = extract_bits(instr, M_START, M_END),
        .rm  = extract_bits(instr, RM_START, RM_END),
        .rn  = extract_bits(instr, RN_START, RN_END),
        .rd  = extract_bits(instr, DP_RD_START, DP_RD_END),
    };
    // Sets fields of multiply instruction
    if (format.M == MULTIPLY_M) {
        format.reg_type = REG_MULTIPLY;
        format.operand.multiply_operand.x = extract_bits(instr, X_START, X_END);
        format.operand.multiply_operand.ra = extract_bits(instr, RA_START, RA_END);
    } else {
        format.operand.arithmetic_logical_operand = extract_bits(instr, OPERAND_START, OPERAND_END);
        uint32_t shift = extract_bits(instr, SHIFT_START, SHIFT_END);
        if (extract_bits(instr, ARITHMETIC_LOGICAL_BIT, ARITHMETIC_LOGICAL_BIT) == ARITHMETIC_BIT) {
            // Sets fields of arithmetic instruction
            format.reg_type = REG_ARITHMETIC;
            format.opr.arithmetic_shift = shift;
        } else {
            // Sets fields of logical instruction
            format.reg_type = REG_LOGICAL;
            format.opr.logical_opr.shift = shift;
            format.opr.logical_opr.N = extract_bits(instr, N_START, N_END);
        }
    }

    // Sets the type and format of the decoded instruction
    decoded->type = DATA_PROCESSING_REG;
    decoded->format.dp_reg_format = format;
}

void execute_dp_reg(Instr *instr, CPUState *cpu) {

    switch (instr->format.dp_reg_format.reg_type) {
        case REG_MULTIPLY:
            execute_dp_reg_multiply(instr, cpu);
            break;
        case REG_ARITHMETIC:
            execute_dp_reg_arithmetic(instr, cpu);
            break;
        case REG_LOGICAL:
            execute_dp_reg_logical(instr, cpu);
            break;
        default:
            // Assume valid instruction - should not reach this case
            assert(0);
    }
    increment_pc(cpu);
}

static void execute_dp_reg_multiply(Instr *instr, CPUState *cpu) {
    DPRegFormat format = instr->format.dp_reg_format;
    uint64_t rn = read_register(format.sf, cpu->registers, format.rn);
    uint64_t rm = read_register(format.sf, cpu->registers, format.rm);
    uint64_t ra = read_register(format.sf, cpu->registers, format.operand.multiply_operand.ra);
    uint64_t product;
    if (format.operand.multiply_operand.x == MULTIPLY_ADD_X) {
        product = rn * rm;
    } else {
        product = - rn * rm;
    }
    write_register(format.sf, cpu->registers, format.rd, ra + product); 
}

static void execute_dp_reg_arithmetic(Instr *instr, CPUState *cpu) {
    DPRegFormat format = instr->format.dp_reg_format;

    uint64_t op2 = arithmetic_logical_shift(instr, cpu);
    DPArithmeticFormat arithmetic_format = {
        format.sf,
        format.opc,
        format.rd,
        format.rn,
        op2,
    };
    execute_dp_arithmetic(&arithmetic_format, cpu);
}

static void execute_dp_reg_logical(Instr *instr, CPUState *cpu) {
    DPRegFormat format = instr->format.dp_reg_format;
    
    // Reads the value of register rn
    uint64_t rn = read_register(format.sf, cpu->registers, format.rn);
    // Performs a shift on the value of register rm to obtain op2
    uint64_t op2 = arithmetic_logical_shift(instr, cpu);
    // Bitwise negates op2 if N bit is set
    if (format.opr.logical_opr.N == NEGATION_N) {
        op2 = ~op2;
    }
    
    // Uses opc as an index to look up and execute the correct logical operation
    uint64_t result = logicTable[format.opc](rn, op2);

    // If operation code is ANDS or BICS, updates the condition flags of PSTATE 
    if (format.opc == SET_FLAGS_OPC) {
        set_flags_logical(instr->format.dp_reg_format.sf, result, cpu);
    }

    // Writes the result of the operation to register rd
    write_register(format.sf, cpu->registers, format.rd, result);
}

static uint64_t arithmetic_logical_shift(Instr *instr, CPUState *cpu) {
    DPRegFormat format = instr->format.dp_reg_format;
    // Gets the shift, depending on whether instruction is arithmetic or logical
    uint8_t shift = format.reg_type == REG_ARITHMETIC ?
        format.opr.arithmetic_shift : format.opr.logical_opr.shift;
    // Gets the correct shift function pointer from the shiftTable and calls it
    uint64_t op2 = shiftTable[shift](
        read_register(format.sf, cpu->registers, format.rm),
        format.operand.arithmetic_logical_operand,
        format.sf);
    return op2;
}

static void set_flags_logical(uint8_t sf, uint64_t result, CPUState *cpu) {
    cpu->pstate.n_flag = sf == BIT_MODE_32 ?
        extract_bits(result, SIGN_BIT_32, SIGN_BIT_32) :
        extract_bits(result, SIGN_BIT_64, SIGN_BIT_64);
    cpu->pstate.z_flag = result == Z_CASE;
    cpu->pstate.c_flag = 0;
    cpu->pstate.v_flag = 0;
}