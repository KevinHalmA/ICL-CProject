#include <stdlib.h>
#include <string.h>

#include "tokeniser.h"

// Delimiters for spliting up a line into tokens for parsing
#define DELIMITERS "., "
// Max number of tokens in a line
#define MAX_TOKENS 10

int tokenise(char *str, TokenisedString *tokenised) {
    // Allocates memory on the heap for an array of char pointers
    tokenised->tokens = malloc(MAX_TOKENS * sizeof(char *));

    // Returns -1 if memory allocation fails
    if (tokenised->tokens == NULL) {
        return -1;
    }

    int count = 0;
    char *token = strtok(str, DELIMITERS);

    // Tokenises the string and stores a pointer to each token in the array
    while (token != NULL) {
        // Allocates memory for the token on the heap
        tokenised->tokens[count] = malloc((MAX_TOKEN_SIZE) * sizeof(char));

        // If allocation fails, frees previously allocated tokens and returns -1
        if (tokenised->tokens[count] == NULL) {
            for (int i = 0; i < count; i++) {
                free(tokenised->tokens[i]);
            }
            return -1;
        }

        // Copies the token string into the array
        strcpy(tokenised->tokens[count], token);

        token = strtok(NULL, DELIMITERS);
        count++;
    }
    
    // Sets the number of tokens
    tokenised->num_tokens = count;

    return 0;
}

void free_tokens(TokenisedString *tokenised) {
    // Iterates through the array to free each individual string token
    for (int i = 0; i < tokenised->num_tokens; i++) {
        free(tokenised->tokens[i]);
    }

    // Frees the array itself
    free(tokenised->tokens);
}