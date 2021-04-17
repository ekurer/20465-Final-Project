#ifndef MMN14_STRUCTS_H
#define MMN14_STRUCTS_H


#include "assembler.h"

/*defining linked list of encoded words and a pointer to that list*/
typedef struct structWords * wordPtr;

/*defining a union that will hold the binary code of the word, depending on it's type*/
typedef union{
    struct{ /*instruction word*/
        unsigned int E:1;
        unsigned int R:1;
        unsigned int A:1;
        unsigned int funct:5;
        unsigned int dest_register:3;
        unsigned int dest_addressing:2;
        unsigned int source_register:3;
        unsigned int source_addressing:2;
        unsigned int opcode:6;
    }mainWord;

    struct{ /*immediate addressing word*/
        unsigned int E:1;
        unsigned int R:1;
        unsigned int A:1;
        unsigned int content:21;
    }word21bits;

    struct{ /*data word*/
        unsigned int content:24;
    }word24bits;
} word;

typedef struct structWords
{
    int address; /* saves the address of the word for the final output */
    int wordType;
    word word_code; /* contains the actual binary code for each word */
    wordPtr next; /* a pointer to the next word in the list */
    wordPtr prev; /* a pointer to the prev word in the list */
} wordMeta;


/* Defining double-linked list of labels and a pointer to that list */
typedef struct structLabels * labelPtr;
typedef struct structLabels {
    char name[LABEL_LENGTH]; /* the name of the label */
    unsigned int address; /* the address of the label */
    enum address_types address_type; /* an enum for address type, whether its stored in 'instruction' or 'data' */
    boolean external; /* a boolean type variable to store if the label is extern or not */
    boolean inActionStatement; /* a boolean type variable to store if the label is in an action statement or not */
    boolean entry; /* a boolean type variable to store if the label is entry or not */
    labelPtr next; /* a pointer to the next label in the list */
} Labels;


/* Defining a linked list to store each time the program uses an extern label, and a pointer to that list */
typedef struct ext * extPtr;
typedef struct ext {
    char name[LABEL_LENGTH]; /* the name of the extern label */
    unsigned int address; /* the address in memory where the external address should be replaced */
    extPtr next; /* a pointer to the next extern in the list */
} ext;


#endif

