#include <stdlib.h>
#include <stdio.h>

#include "assembler.h"
#include "symbol_table.h"

// Expected command-line arguments: paths to input .s file & output .bin file
#define NUM_EXPECTED_ARGUMENTS 3
// Initial symbol table capacity
#define INITIAL_TABLE_CAPACITY 10

/**
 * The entry point of the two-pass assembler program.
 * Opens the assembly source file and binary output file, initialises the symbol
 * table (which maps labels to memory addresses) and runs the 1st and 2nd passes
 * over the assembly file.
 * Exits the program if an error occurs at any point.
 */
int main(int argc, char **argv) {
    // Exits the program if the argument count is invalid
    if (argc != NUM_EXPECTED_ARGUMENTS) {
        fprintf(stderr, "%s\n", "Usage: ./assemble <input_path> <output_path>");
        return EXIT_FAILURE;
    }

    // Opens input assembly file given by 1st command-line argument in read mode
    FILE *in = fopen(argv[1] ,"r");
    if (in == NULL) {
        // Exits the program if null pointer is returned
        perror("Could not open input file");
        return EXIT_FAILURE;
    }

    // Opens output binary file given by 2nd command-line argument in write mode
    FILE *out = fopen(argv[2], "wb");
    if (out == NULL) {
        // Exits the program if null pointer is returned
        perror("Could not open output file");
        return EXIT_FAILURE;
    }

    SymbolTable labels;
    // Initialises symbol table and exits the program if initialisation fails
    if (initialise_table(&labels, INITIAL_TABLE_CAPACITY) != 0) {
        perror("Symbol table could not be initialised");
        return EXIT_FAILURE;
    }

    // Performs the 1st pass over the assembly source code
    first_pass(in, &labels);

    // Performs the 2nd pass over the assembly source code
    second_pass(in, out, &labels);

    // Frees the dynamically allocated memory used for the symbol table
    free_table(&labels);

    // Closes the input file and exits the program if an error occurs
    if (fclose(in) != 0) {
        perror("Could not close input file");
        return EXIT_FAILURE;
    }

    // Closes the output file and exits the program if an error occurs
    if (fclose(out) != 0) {
        perror("Could not close output file");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}