#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getword.h"

char *getRandomWord(Difficulty difficulty) {
    FILE* file;
    char* filename;
    int randomLine;
    int randomFileSelector;
    char word[128];
    int numLines = 0;
    

    //seed random number generator must be called in main program
    //generate random number for file selection
    randomFileSelector = rand() % 100;

    //choose file based on difficulty and random number
    switch(difficulty) {
    case EASY:
        if (randomFileSelector < 60) filename = "short.txt";
        else if (randomFileSelector < 90) filename = "medium.txt";
        else filename = "long.txt";
        break;
    case NORMAL:
        if (randomFileSelector < 25) filename = "short.txt";
        else if (randomFileSelector < 75) filename = "medium.txt";
        else filename = "long.txt";
        break;
    case HARD:
        if (randomFileSelector < 20) filename = "short.txt";
        else if (randomFileSelector < 50) filename = "medium.txt";
        else filename = "long.txt";
        break;
    }
    
    //open file
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return EXIT_FAILURE;
    }
    
    //count lines
    while (fgets(word, sizeof(word), file) != NULL) {
        numLines++;
    }
    
    //get a random line number
    randomLine = rand() % numLines;
    
    //go to random line and get word
    rewind(file); //go back to start of file
    for (int i = 0; i <= randomLine; i++) {
        if (fgets(word, sizeof(word), file) == NULL) {
            //error or end of file
            fprintf(stderr, "%s\n", "randomLine error");
            fclose(file);
            return NULL;
        }
    }
    
    //remove newline character
    word[strcspn(word, "\n")] = 0;
    
    //close file
    fclose(file);
    
    //return a copy of the word (caller is responsible for freeing it)
    char *returnWord = strdup(word);
    if (returnWord == NULL){
        fprintf(stderr, "%s\n", "Memory alloc error for return value");
    }
    return returnWord;
}

int main(void){
    //test getRandomWord
    printf("Testing getRandomWord...\n");
    Difficulty difficulties[] = {EASY, NORMAL, HARD};
    const char* difficulty_names[] = {"EASY", "NORMAL", "HARD"};
    //calling srand in the main program
    srand(time(NULL));
    for (int i = 0; i < 3; i++) {
        printf("Difficulty: %s\n", difficulty_names[i]);
        
        for (int j = 0; j < 10; j++){
            char* word = getRandomWord(difficulties[i]);
            if (word != NULL) {
                printf("Random word: %s\n", word);
                free(word);
            } else {
                printf("An error occurred\n");
            }
        }
    }
    
    return 0;
}
