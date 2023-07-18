#include <stdlib.h>
#include <stdint.h>

#include "registers.h"
#include "../common/utilities.h"

uint64_t read_register(BitMode mode, uint64_t *registers, uint32_t index) {
    // If index encodes the zero register, return 0 (reads from ZR return 0)
    if (index == ZERO_REG_INDEX) {
        return ZERO_REG_VAL;
    }

    uint64_t value = registers[index];
    // If in 32-bit mode, truncates value to lower 32 bits (sets upper to 0)
    if (mode == BIT_MODE_32) {
        value = truncate_32_bits(value);
    }
    return value;
}

void write_register(BitMode mode, uint64_t *registers, uint32_t index, uint64_t value) {
    // If index does not encode the zero register, writes to the given register
    if (index != ZERO_REG_INDEX) {
        // If in 32-bit mode, writes to lower 32 bits and clears upper bits to 0
        if (mode == BIT_MODE_32) {
            registers[index] = truncate_32_bits(value);
        } else {
            registers[index] = value;
        }
    }
}