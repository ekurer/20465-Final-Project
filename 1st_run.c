#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "externs.h"
#include "utilities.h"
#include "prototypes.h"
#include "structs.h"


/* function to manage all activities in the first run */
void first_pass(FILE *fp)
{
    char line[LINE_LENGTH]; /* contains each line analyzed a time */
    int line_num = 1; /* line count */
    head_word = word_list; /* initializing linked list */
    head_data_word = data_word_list;

    /* data and instructions image initializing */
    ic = MEMORY_START;
    dc = 0;

    while(fgets(line, LINE_LENGTH, fp) != NULL) /* read lines until the end of file */
    {
        err = NO_ERROR;  /* reset the error global variable */
        if(!ignore(line)) { /* ignore line if blank or starts with semicolon */
            read_line(line);
        }
        if(is_error()) {
            was_error = TRUE;
            write_error(line_num); /* print a detailed error message */
        }
        line_num++;
    }

    symbols_table = update_data_symbol_address();
    data_word_list = update_dataimg_address(&head_data_word); /* add ic values to data img word addresses*/
    *word_list = *data_word_list; /* link instructions img to data img */
}


/* function to analyze each line at a time */
void read_line(char *line)
{
    /* initializing variables for the type of the directive/command */
    int dir_type = UNKNOWN_DIRECTIVE;
    int command_type = UNKNOWN_DIRECTIVE;


    boolean label = FALSE; /* true if this there's a label in the line */
    labelPtr label_node = NULL; /* will contain the label if exists */

    char current_token[LINE_LENGTH]; /* will hold the token being analyzed */

    line = skip_spaces(line); /* skip to the next non-blank/whitespace character */
    if(end_of_line(line)) return; /* no error in case of blank line */
    if(!isalpha(*line) && *line != '.') /* first non-blank character must be dot or letter */
    {
        err = SYNTAX_ERR;
        return;
    }

    tokenize(current_token, line); /* assuming that label is separated from other tokens by a whitespace */
    if(is_label(current_token, COLON)){ /*check if the first token is a label that contains a colon */
        label = TRUE;
        label_node = add_label(&symbols_table, current_token, ic, FALSE, FALSE); /* adding the label to the global symbols table */
        if(label_node == NULL) /* any kind of error in creating the label */
            return;
        line = next_token(line); /* check the next token */
        if(end_of_line(line))
        {
            err = LABEL_ONLY; /* a line can't be only a label */
            return;
        }
        tokenize(current_token,line); /* continue to next token */
    }

    if(is_error()) /* if is_label returns an error */
        return;

    if((dir_type = which_directive(current_token)) != NOT_FOUND) /* analyzing token to see if it's a directive and what type */
    {
        if(label)
        {
            if(dir_type == EXTERN || dir_type == ENTRY){ /* if entry/extern come after a label, delete the label */
                delete_label(&symbols_table, label_node->name);
                label=FALSE;
            }
            else
            {
                label_node -> address = dc; /* address of data label is dc */
                label_node->address_type = DATA_IMG;
            }
        }
        line = next_token(line);
        handle_directive(dir_type,line);
    }

    else if ((command_type = which_command(current_token)) != NOT_FOUND) /* in case of a command, analyzing it's type */
    {
        if(label)
        {
            label_node -> inActionStatement = TRUE;
            label_node -> address = ic;
            label_node -> address_type = INST_IMG;
        }
        line = next_token(line);
        handle_command(command_type,line);
    }

    else
    {
        err = COMMAND_NOT_FOUND;
    }

}

/*  this function handles all directives (.data, .string, .entry, .extern) */
int handle_directive(int type, char *line)
{
    if(line == NULL || end_of_line(line)) /*  all directives must have at least one parameter */
    {
        err = DIRECTIVE_NO_PARAMS;
        return ERROR;
    }

    switch (type)
    {
        case DATA:
            /*  handle .data directive and insert values separated by comma to the memory */
            return handle_data_directive(line);

        case STRING:
            /*  handle .string directive and insert all characters (including '\0') to memory */
            return handle_string_directive(line);

        case ENTRY:
            /*  check only for syntax of entry (should not contain more than one parameter) */
            if(!end_of_line(next_token(line))) /*  if there's another (after the first one) */
            {
                err = DIRECTIVE_INVALID_NUM_PARAMS;
                return ERROR;
            }
            break;

        case EXTERN:
            /*  Handle .extern directive */
            return handle_extern_directive(line);
    }
    return NO_ERROR;
}

/* this function will analyze the commands, given it's type and characters.
it will identify the addressing methods of the operands
and will encode of the command to the IC */
int handle_command (int type, char *line) {
    boolean is_first = FALSE, is_second = FALSE; /* will indicate which operands were received */
    int first_method = 0, second_method = 0; /* will hold the addressing methods of the operands */
    char first_op[32], second_op[32]; /* will hold the operands */

    /* trying to parse two operands */
    line = next_list_token(first_op, line);
    if (!end_of_line(first_op)) /* checking if first operand exists */
    {
        is_first = TRUE; /* first operand exists */
        line = next_list_token(second_op, line);
        if (!end_of_line(second_op)) /* if second operand (should hold temporarily a comma) is not empty */
        {
            if (second_op[0] != ',') /* comma must separate two operands of a command */
            {
                err = COMMAND_UNEXPECTED_CHAR;
                return ERROR;
            } else {
                line = next_list_token(second_op, line);
                if (end_of_line(second_op)) /* if second operand is not empty */
                {
                    err = COMMAND_UNEXPECTED_CHAR;
                    return ERROR;
                }
                is_second = TRUE; /* second operand exists */
            }
        }
    }
    line = skip_spaces(line);
    if (!end_of_line(line)) /* if the line continues after two operands */
    {
        err = COMMAND_TOO_MANY_OPERANDS;
        return ERROR;
    }

    if (is_first)
        first_method = detect_method(first_op); /* detect addressing method of first operand */

    if (is_second)
        second_method = detect_method(second_op); /* detect addressing method of second operand */

    if (!is_error()) /* if no error found when parsing addressing methods */
    {
        if (command_accept_num_operands(type, is_first, is_second)) /* if number of operands per command are legal */
        {
            if (command_accept_methods(type, first_method, second_method)) { /* if addressing methods per command are legal */

                    /* encode command to words in the memory and increase ic by the number of words */
                    encode_word(type, is_first, is_second, first_method, second_method, first_op, second_op);
                }

                else
                {
                    err = COMMAND_INVALID_OPERANDS_METHODS;
                    return ERROR;
                }
            } else {
                err = COMMAND_INVALID_NUMBER_OF_OPERANDS;
                return ERROR;
            }
        }

        return NO_ERROR;

    }

/* this function handles a .string directive by analyzing it and encoding it */
int handle_string_directive(char *line)
{
    char token[LINE_LENGTH];

    line = next_token_string(token, line);
    if(!end_of_line(token) && is_string(token)) { /* if token exists and it's a valid string */
        line = skip_spaces(line);
        if(end_of_line(line)) /* if there's no additional token */
        {
            /* removing quotation marks and encoding the string to data */
            token[strlen(token) - 1] = '\0';
            write_string_to_data(token + 1);
        }

        else /*  there's another token */
        {
            err = STRING_TOO_MANY_OPERANDS;
            return ERROR;
        }

    }

    else /* invalid string */
    {
        err = STRING_OPERAND_NOT_VALID;
        return ERROR;
    }

    return NO_ERROR;
}

/* this function handles a .data directive by analyzing it and encoding it  */
int handle_data_directive(char *line)
{
    char token[32]; /* holds tokens */

    /* these booleans mark if there was a number or a comma before current token,
     so that if there wasn't a number, then a number will be required and
     if there was a number but not a comma, a comma will be required */
    boolean number = FALSE, comma = FALSE;

    while(!end_of_line(line))
    {
        line = next_list_token(token, line); /* getting current token */

        if(strlen(token) > 0) /* not an empty token */
        {
            if (!number) { /* if there wasn't a number before */
                if (!is_number(token)) { /* then the token must be a number */
                    err = DATA_EXPECTED_NUM;
                    return ERROR;
                }

                else {
                    number = TRUE; /* if a valid number was inputted */
                    comma = FALSE; /* resetting comma (now it is needed) */
                    write_num_to_data(atoi(token)); /*  encoding number to data */
                }
            }

            else if (*token != ',') /* if there was a number, a comma is needed */
            {
                err = DATA_EXPECTED_COMMA_AFTER_NUM;
                return ERROR;
            }

            else /* if there was a comma, it should be only once (comma should be false) */
            {
                if(comma) {
                    err = DATA_COMMAS_IN_A_ROW;
                    return ERROR;
                }
                else {
                    comma = TRUE;
                    number = FALSE;
                }
            }

        }
    }
    if(comma == TRUE)
    {
        err = DATA_UNEXPECTED_COMMA;
        return ERROR;
    }
    return NO_ERROR;
}

/*  this function encodes a given number to the linked list of words */
void write_num_to_data(int num)
{
    /* inserting a word that contains the value of a number, and attaching current dc as address */
    data_word_list->address = dc++;
    data_word_list->wordType = BIT24_WORD;
    data_word_list->word_code.word24bits.content = num;

    add_new_data_node();
}


/* this function encodes a given string to data */
void write_string_to_data(char *str)
{
    while(!end_of_line(str))
    {
        /* inserting a word that contains the value of a character, and attaching current dc as address */
        data_word_list->address = dc++;
        data_word_list->wordType = BIT24_WORD;
        data_word_list->word_code.word24bits.content = (unsigned int) *str;

        add_new_data_node();
        str++;
    }

    /* insert a word with the value for end of string */
    data_word_list->address = dc++;
    data_word_list->wordType = BIT24_WORD;
    data_word_list->word_code.word24bits.content = '\0';

    add_new_data_node();
}

/* this function tries to find the addressing method of a given operand and returns -1 if it was not found */
int detect_method(char * operand)
{
    if(end_of_line(operand)) return NOT_FOUND;

    /*Immediate addressing* method check (0) */
    if (*operand == '#') { /* First character is '#' */
        operand++;
        if (is_number(operand))
            return IMMEDIATE_ADDRESS;
    }

    /*Register addressing* method check (3) */
    else if (is_register(operand))
        return REGISTER_ADDRESS;

    /*Direct addressing* method check (1) */
    else if (is_label(operand, FALSE)) /* Checking if it's a label when there shouldn't be a colon (:) at the end */
        return DIRECT_ADDRESS;

    /*Relative addressing* method check (2) */
    else if (*operand == '&') { /* First character is '&' */
        operand++;
        if (is_label(operand, FALSE))
            return RELATIVE_ADDRESS;
    }
    err = COMMAND_INVALID_METHOD;
    return NOT_FOUND;
}

/* this function checks for the validity of given addressing methods according to the opcode */
boolean command_accept_methods(int type, int first_method, int second_method)
{
    switch (type)
    {
        /* These opcodes only accept
         * Source: 0, 1, 3
         * Destination: 1, 3
         */
        case MOV:
        case ADD:
        case SUB:
            return (first_method == IMMEDIATE_ADDRESS ||
                    first_method == DIRECT_ADDRESS ||
                    first_method == REGISTER_ADDRESS)
                   &&
                   (second_method == DIRECT_ADDRESS ||
                    second_method == REGISTER_ADDRESS);

            /* CMP opcode only accept
             * Source: 0, 1, 3
             * Destination: 0, 1, 3
            */
        case CMP:
            return (first_method == IMMEDIATE_ADDRESS ||
                    first_method == DIRECT_ADDRESS ||
                    first_method == REGISTER_ADDRESS)
                   &&
                   (second_method == IMMEDIATE_ADDRESS ||
                    second_method == DIRECT_ADDRESS ||
                    second_method == REGISTER_ADDRESS);

            /* LEA opcode only accept
             * Source: 1
             * Destination: 1, 3
            */
        case LEA:
            return (first_method == DIRECT_ADDRESS)
                   &&
                   (second_method == DIRECT_ADDRESS ||
                    second_method == REGISTER_ADDRESS);

            /* These opcodes only accept
             * Source: NONE
             * Destination: 1, 3
            */
        case CLR:
        case NOT:
        case INC:
        case DEC:
        case RED:
            return first_method == DIRECT_ADDRESS ||
                   first_method == REGISTER_ADDRESS;

            /* These opcodes only accept
            * Source: NONE
            * Destination: 1, 2
           */
        case JMP:
        case BNE:
        case JSR:
            return first_method == DIRECT_ADDRESS ||
                   first_method == RELATIVE_ADDRESS;

            /* These opcodes only accept
             * Source: NONE
             * Destination: 0, 1, 3
            */
        case PRN:
            return first_method == IMMEDIATE_ADDRESS ||
                   first_method == DIRECT_ADDRESS ||
                   first_method == REGISTER_ADDRESS;

            /* These opcodes are always ok because they accept all methods/none of them and
              number of operands is being verified in another function */
        case RTS:
        case STOP:
            return TRUE;
    }
    return FALSE;
}

/* this function checks for the validity of given methods according to the opcode */
boolean command_accept_num_operands(int type, boolean first, boolean second)
{
    switch (type)
    {
        /* These opcodes must receive 2 operands */
        case MOV:
        case CMP:
        case ADD:
        case SUB:
        case LEA:
            return first && second;

            /* These opcodes must only receive 1 operand */
        case CLR:
        case NOT:
        case INC:
        case DEC:
        case JMP:
        case BNE:
        case JSR:
        case RED:
        case PRN:
            return first && !second;

            /* These opcodes can't have any operand */
        case RTS:
        case STOP:
            return !first && !second;
    }
    return FALSE;
}

/* this function will encode the words based on one instruction line
 * up to three words per line */
void encode_word(int type, int is_first, int is_second, int first_method, int second_method,
                 char *first_op, char *second_op){

    char *err_ptr;
    int tmpmethod = -1;

    /* build first word, out of three */
    {
    word_list->address = ic++;
    word_list->wordType = MAIN_WORD;

    if (is_first && !is_second) {
        is_first = FALSE;
        is_second = TRUE;
        tmpmethod = first_method;
        first_method = second_method;
        second_method = tmpmethod;
        second_op = first_op;
        first_op = "\0";
    }

    word_list->word_code.mainWord.opcode = find_opcode(type);
    word_list->word_code.mainWord.source_addressing = (is_first ? first_method : 0);
    word_list->word_code.mainWord.source_register = register_value(first_op);
    word_list->word_code.mainWord.dest_addressing = (is_second ? second_method : 0);
    word_list->word_code.mainWord.dest_register = register_value(second_op);
    word_list->word_code.mainWord.funct = find_funct(type);
    word_list->word_code.mainWord.A = 1;
    word_list->word_code.mainWord.R = 0;
    word_list->word_code.mainWord.E = 0;

    add_new_inst_node();

    }

    /* build optional second word, out of three */
    if(is_first) {
        if (first_method == IMMEDIATE_ADDRESS) { /* encode the number in the operand to the word */
            word_list->address = ic++;
            word_list->wordType = BIT21_WORD;
            first_op+= 1; /* removes the hashtag */
            word_list->word_code.word21bits.content = strtol(first_op, &err_ptr, 10);
            word_list->word_code.word21bits.A = 1;
            word_list->word_code.word21bits.R = 0;
            word_list->word_code.word21bits.E = 0;
            add_new_inst_node();
        }
        else if (first_method == DIRECT_ADDRESS || first_method == RELATIVE_ADDRESS ) { /* create an empty word that will be filled with address of label in 2nd run */
            word_list->address = ic++;
            word_list->wordType = 0;
            word_list->word_code.word21bits.content = 0;
            word_list->word_code.word21bits.A = 0;
            word_list->word_code.word21bits.R = 0;
            word_list->word_code.word21bits.E = 0;
            add_new_inst_node();
        }
    }

    /* build optional third word, out of three */
    if(is_second) {
        if (second_method == IMMEDIATE_ADDRESS) { /* encode the number in the operand to the word */
            word_list->address = ic++;
            word_list->wordType = BIT21_WORD;
            second_op+= 1; /* removes the hashtag */
            word_list->word_code.word21bits.content = strtol(second_op, &err_ptr, 10);
            word_list->word_code.word21bits.A = 1;
            word_list->word_code.word21bits.R = 0;
            word_list->word_code.word21bits.E = 0;
            add_new_inst_node();
        }
        if (second_method == DIRECT_ADDRESS || second_method == RELATIVE_ADDRESS) { /* create an empty word that will be filled with address of label in 2nd run */
            word_list->address = ic++;
            word_list->wordType = 0;
            word_list->word_code.word21bits.content = 0;
            word_list->word_code.word21bits.A = 0;
            word_list->word_code.word21bits.R = 0;
            word_list->word_code.word21bits.E = 0;
            add_new_inst_node();
        }
    }


}

/* this function returns the opcode value of the given command */
int find_opcode(int type){

    switch (type)
    {
        case MOV:
            return 0;

        case CMP:
            return 1;

        case ADD:
        case SUB:
            return 2;

        case LEA:
            return 4;

        case CLR:
        case NOT:
        case INC:
        case DEC:
            return 5;

        case JMP:
        case BNE:
        case JSR:
            return 9;

        case RED:
            return 12;

        case PRN:
            return 13;

        case RTS:
            return 14;

        case STOP:
            return 15;
    }
    return -1;
}

/* this function returns the funct value of the given command */
int find_funct(int type){

    switch (type)
    {

        case ADD:
        case CLR:
        case JMP:
            return 1;

        case SUB:
        case NOT:
        case BNE:
            return 2;

        case INC:
        case JSR:
            return 3;

        case DEC:
            return 4;

    }
    return 0;
}

/* this function returns the register number or 0 if it's not a register */
int register_value(char *operand){
    if(is_register(operand))
        return operand[1];
    else
        return 0;
}

/* this function handles an .extern directive */
int handle_extern_directive(char *line)
{
    char token[LABEL_LENGTH]; /* will hold the label */

    tokenize(token, line); /* getting the next token */
    if(end_of_line(token)) /* If the token is empty, then there's no label */
    {
        err = EXTERN_NO_LABEL;
        return ERROR;
    }
    if(!is_label(token, FALSE)) /*  The token should be a label (without a colon) */
    {
        err = EXTERN_INVALID_LABEL;
        return ERROR;
    }

    line = next_token(line);
    if(!end_of_line(line))
    {
        err = EXTERN_TOO_MANY_OPERANDS;
        return ERROR;
    }

    /* trying to add the label to the symbols table */
    if(add_label(&symbols_table, token, EXTERNAL_DEFAULT_ADDRESS, TRUE) == NULL)
        return ERROR;
    return is_error(); /* error code might be 1 if there was an error in is_label() */
}

/* this function checks whether a given token is a label or not
the parameter colon states whether the function should look for a ':' or not
when parsing parameter (to make it easier for both kinds of tokens passed to this function) */
boolean is_label(char *token, int colon)
{
    boolean has_digits = FALSE; /* if there are digits inside the label, we can easily skip checking if it's a command name. */

    int token_len = strlen(token);
    int i;

    /* checking if token's length is not too short */
    if(token == NULL ||
       token_len < (colon ? MINIMUM_LABEL_LENGTH_WITH_COLON: MINIMUM_LABEL_LENGTH_WITHOUT_COLON))
        return FALSE;

    if(colon && token[token_len - 1] != ':') return FALSE; /* if colon = TRUE, there must be a colon at the end */

    if (token_len > LABEL_LENGTH) {
        if(colon) err = LABEL_TOO_LONG; /* it's an error only if we search for a label definition */
        return FALSE;
    }
    if(!isalpha(*token)) { /* first character must be a letter */
        if(colon) err = LABEL_INVALID_FIRST_CHAR;
        return FALSE;
    }

    if (colon) {
        token[token_len - 1] = '\0'; /* the following part is more convenient without a colon */
        token_len--;
    }

    /* check if all characters are digits or letters */
    for(i = 1; i < token_len; i++) /* we have already checked if the first character is ok */
    {
        if(isdigit(token[i]))
            has_digits = TRUE;
        else if(!isalpha(token[i])) {
            /* it's not a label but it's an error only if someone put a colon at the end of the token */
            if(colon) err = LABEL_ONLY_ALPHANUMERIC;
            return FALSE;
        }
    }

    if(!has_digits) /* it can't be a command */
    {
        if (which_command(token) != NOT_FOUND) {
            if(colon) err = LABEL_CANT_BE_COMMAND; /* Label can't have the same name as a command */
            return FALSE;
        }
    }

    if(is_register(token)) /* final obstacle: it's a label only if it's not a register */
    {
        if(colon) err = LABEL_CANT_BE_REGISTER;
        return FALSE;
    }

    return TRUE;
}
