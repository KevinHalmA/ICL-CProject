#include <stdlib.h>

#include "binary_loader.h"
#include "emulator.h"

// Expected command-line arguments: paths to input .bin file & output .out file
#define NUM_EXPECTED_ARGUMENTS 3

/**
 * The entry point of the emulator program.
 * Initialises the emulator, reads binary object code from a specified .bin file
 * into the emulator memory, and runs the main execution pipeline.
 * Upon termination, writes the emulator state to a specified .out file.
 */
int main(int argc, char **argv) {
    // Exits the program if the argument count is invalid
    if (argc != NUM_EXPECTED_ARGUMENTS) {
        fprintf(stderr, "%s", "Usage: ./emulate <input_path> <output_path>\n");
        return EXIT_FAILURE;
    }

    // Opens binary file given by 1st command-line argument in read binary mode
    FILE *in = open_file(argv[1]);
    // Exits the program if null pointer is returned
    if (in == NULL) {
        fprintf(stderr, "%s", "Input file could not be opened.\n");
        return EXIT_FAILURE;
    }

    // Initialises the emulator and exits the program if initialisation fails
    CPUState cpu;
    if (initialise_emulator(&cpu) != 0) {
        fprintf(stderr, "%s", "Emulator could not be initialised.\n");
        return EXIT_FAILURE;
    }
    
    // Loads the binary file into memory
    if (load_file(in, (&cpu)->memory) != 0) {
        fprintf(stderr, "%s", "Binary file could not be loaded into memory.\n");
        return EXIT_FAILURE;
    }

    // Closes the binary file and exits the program if attempt fails
    if (close_file(in) != 0) {
        fprintf(stderr, "%s", "Error occurred while closing input file");
        return EXIT_FAILURE;
    }

    // Runs the main execution pipeline of the emulator
    run_emulator(&cpu);

    // Opens output file given by 2nd command-line argument in write text mode
    FILE *out = fopen(argv[2], "w");
    // Exits the program if null pointer is returned
    if (out == NULL) {
        fprintf(stderr, "%s", "Output file could not be opened.\n");
        return EXIT_FAILURE;
    }

    // Writes the final emulator state to the output file
    write_output(&cpu, out);

    // Closes the output file and exits the program if attempt fails
    if (close_file(out) != 0) {
        fprintf(stderr, "%s", "Error occurred while closing output file");
        return EXIT_FAILURE;
    }

    // Frees all dynamically allocated memory associated with the emulator
    free_emulator(&cpu);
    
    return EXIT_SUCCESS;
}