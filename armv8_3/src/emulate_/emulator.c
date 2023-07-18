#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include "emulator.h"
#include "../common/utilities.h"
#include "../common/instructions.h"
#include "registers.h"
#include "memory.h"
#include "dp_immediate.h"
#include "dp_register.h"
#include "single_data_transfer.h"
#include "branch.h"

/**
 * Declares a type DecodePtr representing a pointer to a decode function
 */ 
typedef void (*DecodePtr)(uint32_t, Instr *);

/**
 * Declares a key-value pair with key = instruction type, value = pointer to a
 * decode function
 */
typedef struct {
    uint32_t mask;
    uint32_t pattern;
    DecodePtr func_ptr;
} DecodeEntry;

/**
 * Defines a table (array of structs) that maps the op0 bit mask and pattern for
 * an instruction type to a pointer to its corresponding decode function
 */
static DecodeEntry decodeTable[] = {
    {DP_IMM_MASK, DP_IMM_PATTERN, &decode_dp_imm},
    {DP_REG_MASK, DP_REG_PATTERN, &decode_dp_reg},
    {SDT_MASK, SDT_PATTERN, &decode_single_data_transfer},
    {BRANCH_MASK, BRANCH_PATTERN, &decode_branch},
};

/*
 * Declares a type ExecutePtr representing a pointer to an execute function
 */
typedef void (*ExecutePtr)(Instr *, CPUState *);

/**
 * Declares a key-value pair with key = instruction type, value = pointer to an
 * execute function
 */
typedef struct {
    InstrType type;
    ExecutePtr func_ptr;
} ExecuteEntry;

/**
 * Defines a table (array of structs) that maps an instruction type to a pointer
 * to its corresponding execute function
 */
static ExecuteEntry executeTable[] = {
    {DATA_PROCESSING_IMM, &execute_dp_imm},
    {DATA_PROCESSING_REG, &execute_dp_reg},
    {SINGLE_DATA_TRANSFER, &execute_single_data_transfer},
    {BRANCH, &execute_branch}
};

/**
 * Fetches and returns the next instruction to be executed, using the address
 * stored in the CPU program counter
 */
static uint32_t fetch(CPUState *);

/**
 * Decodes a 32-bit instruction into an internal representation
 * Pre: the instruction is valid
 */
static void decode(uint32_t, Instr *);

/**
 * Executes a decoded instruction using a pointer to its internal representation
 * and updates the state of the CPU accordingly
 */
static void execute(Instr *, CPUState *);

/**
 * Returns the char representation of a flag - if flag is set, returns specified
 * symbol, otherwise returns unset symbol ('-')
 */
static char show_flag(bool, char);

int initialise_emulator(CPUState *cpu) {
    // Dynamically allocates memory on the heap and initialises all values to 0
    cpu->memory = calloc(MEMORY_SIZE, sizeof(uint8_t));
    // Returns -1 if dynamic memory allocation fails
    if (cpu->memory == NULL) {
        return -1;
    }
    // Initialises the values of the general-purpose registers to 0
    for (int i = 0; i < NUM_GENERAL_REGISTERS; i++) {
        (cpu->registers)[i] = 0;
    }
    // Sets zero register = 0
    cpu->zr = 0;
    // Sets program counter = 0
    cpu->pc = 0;
    // Sets processor state condition flags {N, Z, C, V} = {0, 1, 0, 0}
    PState pstate = { .n_flag = 0, .z_flag = 1, .c_flag = 0, .v_flag = 0 };
    cpu->pstate = pstate;
    // Returns 0 if success 
    return 0;
}

void run_emulator(CPUState *cpu) {
    for(;;) {

        // Fetches the next instruction to be executed
        uint32_t instr = fetch(cpu);

        // Stops execution pipeline when halt instruction is reached
        if (instr == HALT_PATTERN) {
            break;
        }

        if (instr == NOP_PATTERN) {
            // Increments the PC if instruction is nop (no operation) 
            increment_pc(cpu);
        } else {
            // Otherwise, decodes and executes the instruction
            Instr decoded;
            decode(instr, &decoded);
            execute(&decoded, cpu);
        }
    }
}

static uint32_t fetch(CPUState *cpu) {
    return read_memory(BIT_MODE_32, cpu->memory, cpu->pc);
}

static void decode(uint32_t instr, Instr *decoded) {
    // Extracts the bits representing the op0 (bits 25-28)
    uint32_t op0 = extract_bits(instr, OP0_START, OP0_END);

    // Uses decodeTable to match instr type with the correct decode function
    for (int i = 0; i < sizeof(decodeTable) / sizeof(decodeTable[0]); i++) {
        // If bitwise AND of op0 and mask = pattern, instruction type matches
        if ((op0 & decodeTable[i].mask) == decodeTable[i].pattern) {
            // Passes the instruction to its corresponding decode function
            decodeTable[i].func_ptr(instr, decoded);
        }
    }
}

static void execute(Instr *instr, CPUState *cpu) {
    // Uses executeTable to match instr type with the correct execute function
    for (int i = 0; i < sizeof(executeTable) / sizeof(executeTable[0]); i++) {
        if (executeTable[i].type == instr->type) {
            // Passes the instruction to its corresponding execute function
            executeTable[i].func_ptr(instr, cpu);
        }
    } 
}

static char show_flag(bool flag, char symbol) {
    return flag ? symbol : UNSET_SYMBOL;
}

void write_output(CPUState *cpu, FILE *fp) {
    // Writes the value of each general-purpose register
    fprintf(fp, "Registers:\n");
    for (int i = 0; i < NUM_GENERAL_REGISTERS; i++) {
        // %0x2d: Pads int with 0s up to width 2
        // %016lx: Displays long in hexadecimal and pads with 0s up to width 16
        fprintf(fp, "X%02d = %016lx\n", i, cpu->registers[i]);
    }

    // Writes the value of Program Counter 
    fprintf(fp, "PC  = %016lx\n", cpu->pc);

    // Writes the condition flags of PSTATE (eg: 'N' if set, '-' otherwise)
    fprintf(fp, "PSTATE : %c%c%c%c\n",
        show_flag(cpu->pstate.n_flag, N_FLAG_SYMBOL),
        show_flag(cpu->pstate.z_flag, Z_FLAG_SYMBOL),
        show_flag(cpu->pstate.c_flag, C_FLAG_SYMBOL),
        show_flag(cpu->pstate.v_flag, V_FLAG_SYMBOL)
    );

    // Reads memory word by word and writes if the value is non-zero
    fprintf(fp, "Non-Zero memory:\n");
    for (int i = 0; i < MEMORY_SIZE; i += WORD_BITS / CHAR_BIT) {
        uint32_t word = read_memory(BIT_MODE_32, cpu->memory, i);
        if (word != 0) {
            // %08x: Displays int in hexadecimal and pads with 0s up to width 8
            fprintf(fp, "0x%08x : %08x\n", i, word);
        }
    }
}

void free_emulator(CPUState *cpu) {
    free(cpu->memory);
}