#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void filter(const char* dictionary) {
    FILE *dict, *shortFile, *mediumFile, *longFile;
    char word[128];

    //open dictionary file
    dict = fopen(dictionary, "r");
    if (dict == NULL) {
        fprintf(stderr, "Could not open file %s\n", dictionary);
        return EXIT_FAILURE;
    }

    //open output files
    shortFile = fopen("short.txt", "w");
    mediumFile = fopen("medium.txt", "w");
    longFile = fopen("long.txt", "w");

    //if the files couldn't be opened error
    if (shortFile == NULL || mediumFile == NULL || longFile == NULL) {
        fprintf(stderr, "%s\n", "Could not open output files");
        return EXIT_FAILURE;
    }

    //read words from dictionary and write to appropriate files
    while (fgets(word, sizeof(word), dict) != NULL) {

        //remove the newline character
        word[strcspn(word, "\n")] = 0;

        size_t len = strlen(word);
        if (len <= 4) {
            fprintf(shortFile, "%s\n", word);
        } else if (len >= 5 && len <= 7) {
            fprintf(mediumFile, "%s\n", word);
        } else if (len >= 8) {
            fprintf(longFile, "%s\n", word);
        }
    }

    //close all files
    fclose(dict);
    fclose(shortFile);
    fclose(mediumFile);
    fclose(longFile);
}

int main(void) {
    //test filter
    const char* dictionary_file = "dictionary.txt";
    printf("Separating words from %s...\n", dictionary_file);
    filter(dictionary_file);
    printf("Separation completed.\n");

    return 0;
}
