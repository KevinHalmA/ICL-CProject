#include <stdint.h>
#include <assert.h>

#include "utilities.h"

uint64_t get_bit_mask(int start, int end) {
    int length = end - start + 1;
    if (length == BIT_SIZE_64) {
        return ~((uint64_t) 0);
    } else {
        return ((1UL << length) - 1) << start;
    }
}

uint64_t extract_bits(uint64_t value, int start, int end) {
    return (value & get_bit_mask(start, end)) >> start;
}

uint64_t truncate_32_bits(uint64_t value) {
    return value & get_bit_mask(0, BIT_SIZE_32 - 1);
}

int64_t sign_extend(int32_t value, int bit_length) {
    // Implicitly converts to int64_t 
    int64_t extended_value = value;
    // Computes the sign bit (MSB)
    int sign_bit = value >> (bit_length - 1);
    // If sign bit = 1, fill the upper 64 - bit_length bits with 1s
    if (sign_bit == 1) {
        // Creates a mask where the lower 'bit_length bits are 1s 
        uint64_t mask = get_bit_mask(0, bit_length - 1);
        // Performs a bitwise OR with the inverted mask
        extended_value |= ~mask;
    }
    return extended_value;
}

void increment_pc(CPUState *cpu) {
    cpu->pc += INSTR_BYTES;
}

uint64_t calculate_pc_offset(uint64_t pc, int32_t offset, int offset_size) {
    int64_t scaled_offset = sign_extend(offset * INSTR_BYTES, offset_size + 2);
    return (int64_t) pc + scaled_offset;
}

uint64_t logical_shift_left(uint64_t value, int shift_amount, int sf) {
    assert(sf == BIT_MODE_32 || sf == BIT_MODE_64);
    uint64_t shifted = value << shift_amount;
    if (sf == BIT_MODE_32) {
        shifted = truncate_32_bits(shifted);
    }
    return shifted;
}

uint64_t logical_shift_right(uint64_t value, int shift_amount, int sf) {
    assert(sf == BIT_MODE_32 || sf == BIT_MODE_64);
    uint64_t shifted = value >> shift_amount;
    if (sf == BIT_MODE_32) {
        shifted = truncate_32_bits(shifted);
    }
    return shifted;
}

uint64_t arithmetic_shift_right(uint64_t value, int shift_amount, int sf) {
    assert(sf == BIT_MODE_32 || sf == BIT_MODE_64);
    // Gets the size in bits of the operand
    int bit_size = sf == BIT_MODE_32 ? BIT_SIZE_32 : BIT_SIZE_64; 
    int sign = extract_bits(value, bit_size - 1, bit_size - 1);
    uint64_t shifted = value >> shift_amount;
    if (sign == 1) {
        shifted |= get_bit_mask(bit_size - shift_amount, bit_size - 1);
    }
    if (sf == BIT_MODE_32) {
        shifted = truncate_32_bits(shifted);
    }
    return shifted;
}

uint64_t rotate_right(uint64_t value, int shift_amount, int sf) {
    assert(sf == BIT_MODE_32 || sf == BIT_MODE_64);
    // Gets the size in bits of the operand
    int bit_size = sf == BIT_MODE_32 ? BIT_SIZE_32 : BIT_SIZE_64;
    // Shifts value to the right and does bitwise OR with the bits carried out
    uint64_t rotated = (value >> shift_amount) | (value << (bit_size - shift_amount));
    if (sf == BIT_MODE_32) {
        rotated = truncate_32_bits(rotated);
    }
    return rotated;
}