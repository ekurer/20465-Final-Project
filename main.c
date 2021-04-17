
#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "utilities.h"
#include "prototypes.h"

int ic;
int dc;
int err;

/* declare and initialize global structures and vars */
wordMeta word1 = {0};
wordPtr word_list = &word1;
wordPtr head_word;
labelPtr symbols_table;
boolean entry_existence, extern_existence, was_error;

wordMeta dataword1 = {0};
wordPtr data_word_list = &dataword1;
wordPtr head_data_word;


const char *commands[] = {
        "mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne",
        "jsr", "red", "prn", "rts", "stop"
};

const char *directives[] = {
        ".data", ".string", ".entry", ".extern"
};

/* resets the following vars */
void reset_global_vars()
{
    symbols_table = NULL;
    entry_existence = FALSE;
    extern_existence = FALSE;
    was_error = FALSE;
}

/* receives arguments, first is the program and then multiple assembly files (without file extension)
 * and handles flow of program */
int main(int argc, char *argv[]){
    int i;
    char *input_filename;
    FILE *fp;
    for(i = 1; i < argc; i++)
    {
        input_filename = create_file_name(argv[i], FILE_INPUT); /* Appending .as to filename */
        fp = fopen(input_filename, "r");

        if(fp != NULL) { /* If file exists */
            printf("---------- Started %s assembling process ----------\n\n", input_filename);

            reset_global_vars();
            first_pass(fp);

            if (!was_error) { /* proceed to second pass */
                rewind(fp);
                second_run(fp, argv[i]);
            }

            printf("\n\n---------- Finished %s assembling process ----------\n\n", input_filename);
        }
        else write_error(CANNOT_OPEN_FILE);
        free(input_filename);
        fclose(fp);
    }
    return 0;
}
