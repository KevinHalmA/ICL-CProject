#ifndef BINARY_LOADER_H
#define BINARY_LOADER_H

#include <stdio.h>
#include <stdint.h>

/**
 * Opens a binary file using a given file path - returns a pointer to the
 * associated file stream
 */
extern FILE *open_file(char[]);

/**
 * Closes a binary file - returns 0 if success and -1 otherwise
 */
extern int close_file(FILE *);

/**
 * Loads a binary file into byte-addressable memory given by a pointer - returns
 * 0 if success and -1 otherwise
 */
extern int load_file(FILE *, uint8_t *);

#endif