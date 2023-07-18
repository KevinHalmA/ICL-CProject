#include <stdint.h>
#include <limits.h>

#include "memory.h"
#include "../common/utilities.h"

uint64_t read_memory(BitMode mode, uint8_t *memory, uint64_t address) {
    // Number of bytes to read: 4 bytes in 32-bit mode, 8 bytes in 64-bit mode
    int bytes = (mode == BIT_MODE_32 ? BIT_SIZE_32 : BIT_SIZE_64) / CHAR_BIT;

    uint64_t value = 0;
    // Reads memory one byte at a time
    for (int i = 0; i < bytes; i++) {
        // Since memory is little-endian, shifts each byte i by 8 * i bits
        value |= ((uint64_t) memory[address + i]) << (i * CHAR_BIT);
    }

    return value;
}

void write_memory(BitMode mode, uint8_t *memory, uint64_t address, uint64_t value) {
    // Number of bytes to write: 4 bytes in 32-bit mode, 8 bytes in 64-bit mode
    int bytes = (mode == BIT_MODE_32 ? BIT_SIZE_32 : BIT_SIZE_64) / CHAR_BIT;
    
    // Creates a mask that isolates the least-significant byte
    uint64_t byte_mask = get_bit_mask(0, CHAR_BIT - 1);
    // Writes to memory one byte at a time
    for (int i = 0; i < bytes; i++) {
        // Writes least-significant byte to lowest address (since little-endian)
        memory[address + i] = value & byte_mask;
        // Shifts value to the right by one byte
        value >>= CHAR_BIT;
    }
}