#ifndef TOKENISER_H
#define TOKENISER_H

// Max number of characters in a token
#define MAX_TOKEN_SIZE 15

/**
 * Represents the result of tokenisation - an array of string tokens and the
 * number of tokens
 */
typedef struct {
    char **tokens;
    int num_tokens;
} TokenisedString;

/*
 * Splits up a string into an array of string tokens to be parsed
 * Returns -1 if memory allocation fails, 0 otherwise
 */
extern int tokenise(char *, TokenisedString *);

/*
 * Frees the memory dynamically allocated for an array of string tokens
 */
extern void free_tokens(TokenisedString *);

#endif
