#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Defines the different bit sizes, the position of the sign bit for each bit
 * size and a 32 bit mask
*/
#define BIT_SIZE_32 32
#define SIGN_BIT_32 31
#define BIT_SIZE_64 64
#define SIGN_BIT_64 63

#define MEMORY_SIZE 2097152 // Size of the ARMv8 memory (2MB) in bytes
#define REGISTER_SIZE 64 // Size of a general-purpose register in bits
#define NUM_GENERAL_REGISTERS 31 // Number of general-purpose registers
#define WORD_BITS 32 // Size of a word in bits
#define INSTR_BYTES 4 // Represents the size of an instruction in bytes

#define ZERO_REG_INDEX 31 // The index of the zero register
#define ZERO_REG_VAL 0 // The value of the zero register

/**
 * Represents the symbols of each of the 4 flags, along with a symbol to
 * represent an unset flag
*/
#define N_FLAG_SYMBOL 'N'
#define Z_FLAG_SYMBOL 'Z'
#define C_FLAG_SYMBOL 'C'
#define V_FLAG_SYMBOL 'V'
#define UNSET_SYMBOL '-'

/** 
 * Represents the 4 condition flags of the Processor State (PSTATE) register:
 * n_flag: Negative (result = negative)
 * z_flag: Zero (result = 0)
 * c_flag: Carry (carry/borrow bit was produced)
 * v_flag: oVerflow (signed overflow/underflow)
 */
typedef struct {
    bool n_flag;
    bool z_flag;
    bool c_flag;
    bool v_flag;
} PState;

/**
 * Represents the state of the CPU in an ARMv8 machine:
 * memory:    Pointer to memory block representing byte-addressable ARMv8 memory
 * registers: An array representing 64-bit general purpose registers
 * zr:        Represents the (64-bit) Zero Register 
 * pc:        Represents the (64-bit) Program Counter
 * pstate:    Represents the Processor State register
 */ 
typedef struct {
    uint8_t *memory;
    uint64_t registers[NUM_GENERAL_REGISTERS];
    uint64_t zr;
    uint64_t pc;
    PState pstate;
} CPUState;

/**
 * Represents the bit mode of operations - either 32 or 64 bit
 */
typedef enum {
    BIT_MODE_32 = 0,
    BIT_MODE_64,
} BitMode;

/**
 * Returns a 64-bit mask where bits 'start' to 'end' are set to 1 (the rest = 0)
 */
extern uint64_t get_bit_mask(int, int);

/**
 * Extracts bits 'start' to 'end' (inclusive) from a 32-bit unsigned int
 */
extern uint64_t extract_bits(uint64_t, int, int);

/**
 * Truncates a 64-bit value to 32 bits by clearing the upper 32 bits to 0
*/
extern uint64_t truncate_32_bits(uint64_t);

/**
 * Sign extends a signed value of a given bit length to 64 bits
 */
extern int64_t sign_extend(int32_t, int);

/**
 * Increments the PC register by 4 (the size of an instruction in bytes)
 */
extern void increment_pc(CPUState *);

/**
 * Calculates new PC value after adding the scaled offset
*/
extern uint64_t calculate_pc_offset(uint64_t, int32_t, int);

/**
 * Logically shifts a 32-bit or 64-bit value left by a shift amount
 */
extern uint64_t logical_shift_left(uint64_t, int, int);

/**
 * Logically shifts a 32-bit or 64-bit value right by shift amount
 */
extern uint64_t logical_shift_right(uint64_t, int, int);

/**
 * Arithmetically shifts a 32-bit or 64-bit value right by shift amount
 */
extern uint64_t arithmetic_shift_right(uint64_t, int, int);

/**
 * Rotates a 32-bit or 64-bit value right by shift amount
 */
extern uint64_t rotate_right(uint64_t, int, int);

#endif