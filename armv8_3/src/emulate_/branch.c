#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "branch.h"
#include "../common/utilities.h"
#include "../common/instructions.h"
#include "registers.h"

/**
 * Executes a decoded unconditional branch instruction
 */
static void execute_unconditional(Instr *, CPUState *);

/**
 * Executes a decoded register branch instruction
 */ 
static void execute_register(Instr *, CPUState *);

/**
 * Executes a decoded conditional branch instruction
 * Returns a boolean representing whether the branch condition was satisfied
 */
static bool execute_conditional(Instr *, CPUState *);

/**
 * For a given condition code, determines whether its corresponding condition
 * holds using the condition flags of the PSTATE register:
 * EQ: Z == 1
 * NE: Z == 0
 * GE: N == V
 * LT: N != V
 * GT: Z == 0 && N == V
 * LE: !(Z == 0 && N == V)
 * AL: always holds
 */
static int evaluate_condition(uint8_t, CPUState *);

void decode_branch(uint32_t instr, Instr *decoded) {
    BranchFormat format;

    // Identifies the branch subtype (uncond, register, cond) using bits 30-31
    switch (extract_bits(instr, BRANCH_IDENTIFIER_START, BRANCH_IDENTIFIER_END)) {
        case UNCONDITIONAL_IDENTIFIER:
            // Sets fields for Unconditional Branch type
            format.branch_type = UNCONDITIONAL;
            format.Unconditional.simm26 = extract_bits(instr, SIMM26_START, SIMM26_END);
            break;
        case REGISTER_IDENTIFIER:
            // Sets fields for Register Branch type
            format.branch_type = REGISTER;
            format.Register.xn = extract_bits(instr, XN_START, XN_END);
            break;
        case CONDITIONAL_IDENTIFIER:
            // Sets fields for Conditional Branch type
            format.branch_type = CONDITIONAL;
            format.Conditional.simm19 = (int32_t) extract_bits(instr, SIMM19_START, SIMM19_END);
            format.Conditional.cond = extract_bits(instr, COND_START, COND_END);
            break;
    }

    // Sets the type and format of the decoded instruction
    decoded->type = BRANCH;
    decoded->format.branch_format = format;
}

void execute_branch(Instr *instr, CPUState *cpu) {
    switch (instr->format.branch_format.branch_type) {
        case UNCONDITIONAL:
            execute_unconditional(instr, cpu);
            break;
        case REGISTER:
            execute_register(instr, cpu);
            break;
        case CONDITIONAL:
            // Increments the PC as normal if branch condition was not satisfied
            if (!execute_conditional(instr, cpu)) {
                increment_pc(cpu);
            }
            break;
        default:
            // Assume valid instruction - should not reach this case 
            assert(0);
    }
}

static void execute_unconditional(Instr *instr, CPUState *cpu) {
    // Adds the offset simm26 * 4 to the program counter
    int32_t offset = instr->format.branch_format.Unconditional.simm26;
    cpu->pc = calculate_pc_offset(cpu->pc, offset, SIMM26_LENGTH);
}

static void execute_register(Instr *instr, CPUState *cpu) {
    // Sets the program counter to the address stored in register Xn
    cpu->pc = read_register(BIT_SIZE_64, cpu->registers, instr->format.branch_format.Register.xn);
}

static bool execute_conditional(Instr *instr, CPUState *cpu) {
    // Adds the offset simm19 * 4 to the program counter if condition satisfied
    if (evaluate_condition(instr->format.branch_format.Conditional.cond, cpu)) {
        int32_t offset = instr->format.branch_format.Conditional.simm19;
        cpu->pc = calculate_pc_offset(cpu->pc, offset, SIMM19_LENGTH);
        // Returns true if branch condition is satisfied
        return true;
    }
    return false;
}

static int evaluate_condition(uint8_t cond, CPUState *cpu) {
    int result = 0;
    // Identifies the condition code and sets the result using the PSTATE flags
    switch (cond) {
        case EQ:
            result = cpu->pstate.z_flag == 1;
            break;
        case NE:
            result = cpu->pstate.z_flag == 0;
            break;
        case GE:
            result = cpu->pstate.n_flag == cpu->pstate.v_flag;
            break;
        case LT:
            result = cpu->pstate.n_flag != cpu->pstate.v_flag;
            break;
        case GT:
            result = evaluate_condition(NE, cpu) && evaluate_condition(GE, cpu);
            break;
        case LE:
            result = !evaluate_condition(GT, cpu);
            break;
        case AL:
            result = 1;
            break;
        default:
            // Assume valid condition code - should not reach this case
            assert(0);
    }
    return result;
}