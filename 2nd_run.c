/*
 * HEADERS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "externs.h"
#include "utilities.h"
#include "prototypes.h"
#include "structs.h"

/* external list is declared only in the second run */
ext ext1;
extPtr ext_list = &ext1;
extPtr head_ext;

void second_run(FILE *fp, char *filename){

    /* resetting global linked lists to start */
    char line[LINE_LENGTH]; /* This string will contain each line at a time */
    int line_num = 1; /* Line numbers start from 1 */
    word_list = head_word;
    head_ext = ext_list;

    while(fgets(line, LINE_LENGTH, fp) != NULL) /* Read lines until end of file */
    {
        err = NO_ERROR;
        if (!ignore(line))
            read_line_2nd(line); /* line parser and decision maker */
        if (err != NO_ERROR) { /* print lines and error */
            printf("%d - 'n", line_num);
            write_error(line_num);
        }
        line_num++;
    }

    if (err == NO_ERROR) /* write output files only if no err raised along the program */
        write_output_files(filename);

    /* Free dynamic allocated elements */
    free_labels(&symbols_table);
/*   free_ext(&(head_ext->next)); */
/*    free_words(&(head_word->next)); */
}

void read_line_2nd(char *line) {
    char curr_token[LINE_LENGTH];
    int directive_name = -1 ;
    int command_name = -1;

    line = skip_spaces(line); /* Proceeding to first non-blank character */
    if (end_of_line(line)) return;
    tokenize(curr_token, line);  /* parsing line to tokens proccess */
    if (is_label_declaration(curr_token)) {
        line = next_token(line);
        tokenize(curr_token, line);
    }

    if ((directive_name = which_directive(curr_token)) != NOT_FOUND) {
        /* in the second pass only the entry directive is relevant, to edit entry values*/
        line = next_token(line);
        if (directive_name == ENTRY) {
            tokenize(curr_token, line);
            edit_entry(symbols_table, curr_token);
        }
    }
    else if ((command_name = which_command(curr_token)) != NOT_FOUND) /* Encoding command's additional words */
    {
        line = next_token(line);
        command_manipulation_2nd(command_name, line);
    }
}


/* This function handles commands for the second pass - encoding additional words */
int command_manipulation_2nd(int type, char *line)
{
    char first_op[LINE_LENGTH], second_op[LINE_LENGTH]; /* will hold first and second operands */
    char *src = first_op, *dest = second_op; /* after the check below, src will point to source and
 *                                              dest to destination operands */
    boolean is_src = FALSE, is_dest = FALSE; /* Source/destination operands existence */
    int src_method = UNKNOWN_ADDRESS, dest_method = UNKNOWN_ADDRESS; /* Their addressing methods */
    int is_src_label = 0, is_dest_label =0;

    check_operands_exist(type, &is_src, &is_dest);

    /* Matching src and dest pointers to the correct operands (first or second or both) */
    if(is_src || is_dest)
    {
        line = next_list_token(first_op, line);
        if(is_src && is_dest) /* There are 2 operands */
        {
            line = next_list_token(second_op, line);
            next_list_token(second_op, line);
        }
        else
        {
            dest = first_op; /* If there's only one operand, it's a destination operand */
            src = NULL;
        }
    }

    /* check which operands are labels */
    is_src_label = is_src && is_operand_label(src);
    is_dest_label =is_dest && is_operand_label(dest);


    /* Extracting source and destination addressing methods */
    if (is_src_label) {
        if (src[0] == '&') /* remove '&' from operand to use only the value */
            memmove(src, src + 1, strlen(src));

        proceed_to_blank_word(&word_list); /* reach the next unencoded word to fill with data */
        if (get_label(symbols_table, src)->external) {
            add_label_to_extern(word_list->address, src); /* add to external */
        }
        /* finds the addressing used in its main word */
        src_method = get_from_previous_main_word(SOURCE_ADDRESSING, *word_list);
        encode_additional_words(src, src_method);
    }
    if (is_dest_label) {
        if (dest[0] == '&') /* remove '&' from operand to use only the value */
            memmove(dest, dest + 1, strlen(dest));

        proceed_to_blank_word(&word_list); /* reach the next unencoded word to fill with data */
        if (get_label(symbols_table, dest)->external) {
            add_label_to_extern(word_list->address, dest); /* add to external */
        }
        /* finds the addressing used in its main word */
        dest_method = get_from_previous_main_word(DEST_ADDRESSING, *word_list);
        encode_additional_words(dest, dest_method);
    }
    return 0;

}


/* this function finds for a word node its last main word
 * seeks only in the last 2 since max additional words are 2 */
int get_from_previous_main_word(int field, wordMeta word){
    unsigned address = -1;
    if (word_list->prev->wordType == MAIN_WORD) {
        switch (field) {
            case SOURCE_ADDRESSING:
                address = word.prev->word_code.mainWord.source_addressing;
                break;

            case DEST_ADDRESSING:
                address = word.prev->word_code.mainWord.dest_addressing;
                break;
        }
    }
    else if (word_list->prev->prev->wordType == MAIN_WORD)
        switch (field) {
            case SOURCE_ADDRESSING:
                address = word.prev->prev->word_code.mainWord.source_addressing;
                break;

            case DEST_ADDRESSING:
                address = word.prev->prev->word_code.mainWord.dest_addressing;
                break;
        }
    else
        err = COMMAND_LABEL_DOES_NOT_EXIST;

    return address;
}

/* this method iterates word list to find the next unencoded word */
void proceed_to_blank_word(wordPtr *word){
    while ((*word)->next != NULL && (*word)->wordType != 0) {  /*@@EXC_BAD_ACCESS */
        *word = (*word)->next;
    }
}


/* this method adds external labels to the external linked list */
void add_label_to_extern(unsigned int address, char *label_name){
    extPtr tmpNode = (extPtr)malloc(sizeof(ext));
    memset(tmpNode, 0, sizeof(*tmpNode));
    extern_existence = TRUE;
    ext_list->address = address;
    strcpy(ext_list->name,label_name);
    /* add new node */
    ext_list->next = tmpNode;
    ext_list = ext_list->next;
}

/* this method assigns every field of and additional word based on its properties */
void encode_additional_words(char *operand, int method) {
    unsigned address = 0;
    unsigned distance = 0;
    labelPtr lbl;
    boolean is_external = -1;



    lbl = get_label(symbols_table, operand); /* find the matching label is symbol table */
    if (lbl != NULL) {
        address = lbl->address;
        is_external = lbl->external;

        if (method == DIRECT_ADDRESS) {
            if (is_external == TRUE) {
                word_list->word_code.word21bits.E = 1; /* external get E = 1 */
            } else {
                word_list->word_code.word21bits.R = 1;
            }
            word_list->word_code.word21bits.content = address;
        } else if (method == RELATIVE_ADDRESS) {
            word_list->word_code.word21bits.A = 1;
            distance = address - (word_list->address - 1); /* calculates and assigns the distance
 *                                                          from label address to node address */
            word_list->word_code.word21bits.content = distance;
        }
        word_list->wordType = BIT21_WORD;
    }
    else {
        err = COMMAND_LABEL_DOES_NOT_EXIST;
    }
}

/* This function determines if source and destination operands exist by opcode */
void check_operands_exist(int type, boolean *is_src, boolean *is_dest)
{
    switch (type)
    {
        /* this commands can have both src and dest */
        case MOV:
        case CMP:
        case ADD:
        case SUB:
        case LEA:
            *is_src = TRUE;
            *is_dest = TRUE;
            break;
            /* this commands can have only dest */
        case CLR:
        case NOT:
        case INC:
        case DEC:
        case JMP:
        case BNE:
        case JSR:
        case RED:
        case PRN:
            *is_src = FALSE;
            *is_dest = TRUE;
            break;
            /* this commands get no operands */
        case RTS:
        case STOP:
            *is_src = FALSE;
            *is_dest = FALSE;
    }
}



/* This function writes all 3 output files (if they should be created)*/
int write_output_files(char *original)
{
    FILE *file;

    file = open_file(original, FILE_OBJECT);
    write_output_ob(file);
    printf("Output file at %s%s\n", original,".ob");
    if(entry_existence) {  /* write to output only if there are entries*/
        file = open_file(original, FILE_ENTRY);
        write_output_entry(file);
        printf("Output file at %s%s\n", original,".ent");
    }
    if(extern_existence) /* write to output only if there are entries*/
    {
        file = open_file(original, FILE_EXTERN);
        write_output_extern(file);
        printf("Output file at %s%s\n", original,".ent");
    }

    return NO_ERROR;
}

/* This function opens a file with writing permissions, given the original input filename and the
 * wanted file extension (by type)
 */
FILE *open_file(char *filename, int type)
{
    FILE *file;
    filename = create_file_name(filename, type); /* Creating filename with extension */

    file = fopen(filename, "w"); /* Opening file with permissions */
    free(filename); /* Allocated modified filename is no longer needed */

    if(file == NULL)
    {
        err = CANNOT_OPEN_FILE;
        return NULL;
    }
    return file;
}


/* this function converts a decimal num to binary string */
void b10tob2(char* dst, int n, int num)
{
    int c, d, count;
    char *bin_word = (char*)malloc(24+1);
    count = 0;

    if (bin_word == NULL)
        exit(EXIT_FAILURE);

    /* iterates the num and appends binary result with bit shifting */
    for (c = n-1 ; c >= 0 ; c--)
    {
        d = num >> c;
        if (d & 1)
            *(bin_word+count) = 1 + '0';
        else
            *(bin_word+count) = 0 + '0';
        count++;
    }
    *(bin_word+count) = '\0'; /* append end of string */

    strcat(dst, bin_word);

    free(bin_word);
}


/* this method orchestrates creation of a binary word from a word, while appending each field */
void storeBinary(wordMeta word, char final_bin_word[]) {
    if (word.wordType == MAIN_WORD) {
        b10tob2(final_bin_word, 6, word.word_code.mainWord.opcode);
        b10tob2(final_bin_word, 2, word.word_code.mainWord.source_addressing);
        b10tob2(final_bin_word, 3, word.word_code.mainWord.source_register);
        b10tob2(final_bin_word, 2, word.word_code.mainWord.dest_addressing);
        b10tob2(final_bin_word, 3, word.word_code.mainWord.dest_register);
        b10tob2(final_bin_word, 5, word.word_code.mainWord.funct);
        b10tob2(final_bin_word, 1, word.word_code.mainWord.A);
        b10tob2(final_bin_word, 1, word.word_code.mainWord.R);
        b10tob2(final_bin_word, 1, word.word_code.mainWord.E);
    }
    else if (word.wordType == BIT21_WORD) {
        b10tob2(final_bin_word, 21, word.word_code.word21bits.content);
        b10tob2(final_bin_word, 1, word.word_code.word21bits.A);
        b10tob2(final_bin_word, 1, word.word_code.word21bits.R);
        b10tob2(final_bin_word, 1, word.word_code.word21bits.E);
    }
    else if (word.wordType == BIT24_WORD) {
        b10tob2(final_bin_word, 24, word.word_code.word24bits.content);
    }
}

/* converts binary string to hex num*/
long hexify(char bin_string[]) {
    char *ptr;
    long hex_num;
    hex_num = strtol(bin_string, &ptr, 2);
    return hex_num;
}

/* print ob file with address and word as hex, instructions first and data img after */
void write_output_ob(FILE *fp){
    wordPtr tmpWord = head_word;
    char binary_word[WORD_LENGTH+1] = {0};
    int hexNum;

    fprintf(fp, "%d %d\n", ic-100, dc); /* print the final ic and dc */
    while (tmpWord) {
        memset(binary_word, '\0', sizeof(binary_word));
        storeBinary(*tmpWord, binary_word); /* word fields to binary string */
        hexNum = hexify(binary_word); /* binary string to hex num */
        fprintf(fp, "%07d %06x\n", tmpWord->address, hexNum); /* print to file with predefined length */
        tmpWord = tmpWord->next;
    }
    fclose(fp);
}

/* print ent file: goes over symbol list and prints to file if its an entry */
void write_output_entry(FILE *fp){
    labelPtr tmpLabels = symbols_table;
    while (tmpLabels) {
        if (tmpLabels->entry == TRUE) {
            fprintf(fp, "%s %07d\n", tmpLabels->name, tmpLabels->address);
        }
        tmpLabels = tmpLabels->next;
    }
    fclose(fp);
}

/* prints externals to file: goes over the external linked list and prints every node  */
void write_output_extern(FILE *fp){
    extPtr tmpExterns = head_ext;
    while (tmpExterns->next) {
        fprintf(fp,"%s %07d\n", tmpExterns->name, tmpExterns->address);
        tmpExterns = tmpExterns->next;
    }
    fclose(fp);
}

