#include <stdio.h>
#include <stdint.h>

#include "binary_loader.h"
#include "../common/utilities.h"

/**
 * Returns the size of a file in bytes or -1 if failure
 */
static int get_file_size(FILE *);

FILE *open_file(char filename[]) {
    return fopen(filename, "rb");
}

int close_file(FILE *fp) {
    return fclose(fp);
}

int load_file(FILE *fp, uint8_t *memory) {
    int file_size = get_file_size(fp);
    // Returns -1 if file size is negative or greater than emulator memory size
    if (file_size < 0  || file_size > MEMORY_SIZE) {
        return -1;
    }
    // Reads the file into memory byte by byte
    int num_bytes_read = fread(memory, sizeof(uint8_t), file_size, fp);
    // Returns -1 if an error occurs while reading
    if (num_bytes_read != file_size && ferror(fp)) {
        return -1;
    }
    // Returns 0 if the file has been read successfully
    return 0;
}

static int get_file_size(FILE *fp) {
    // Moves file pointer to the end of the file, returns -1 if fseek fails
    if (fseek(fp, 0, SEEK_END) != 0) {
        return -1;
    }
    // Gets the position of the file pointer relative to the start in bytes
    int size = ftell(fp);
    // Returns -1 if the size is negative
    if (size < 0) {
        return -1;
    }
    // Moves file pointer to the start of the file, returns -1 if fseek fails
    if (fseek(fp, 0, SEEK_SET) != 0) {
        return -1;
    }
    return size;
}