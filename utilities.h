#ifndef MMN14_UTILITIES_H
#define MMN14_UTILITIES_H

/* This file contains prototypes for utilities and helper functions that are being used in the program. */

#include "structs.h"

/* Helper functions that are used for creating files and assigning required extensions to them */
char *create_file_name(char *original, int type);
FILE *open_file(char *filename, int type);

/* Helper functions that are used for parsing tokens and navigating through them */
char *skip_spaces(char *ch);
int ignore(char *line);
int end_of_line(char *line);
char *next_token(char *seq);
void tokenize(char *dest, char *line);
char *next_list_token(char *dest, char *line);
boolean is_label_declaration(char *token);
boolean is_operand_label(char *token);

/* Helper functions that are used to determine types of tokens */
boolean is_string(char *string);
boolean is_number(char *seq);
boolean is_register(char *token);
int which_index(char *token, const char *arr[], int n);
int which_directive(char *token);
int which_command(char *token);
char *next_token_string(char *dest, char *line);
char *next_list_token_string(char *dest, char *line);

/* Functions of symbols table */
labelPtr get_label(labelPtr lbl, char *name);
labelPtr add_label(labelPtr *hptr, char *name, unsigned int address, boolean external, ...);
int edit_entry(labelPtr lbl, char *name);
int delete_label(labelPtr *hptr, char *name);
void free_labels(labelPtr *hptr);
labelPtr update_data_symbol_address();
boolean is_existing_label(labelPtr h, char *name);
void print_labels(labelPtr lbl);

/* Linked list prototypes */
void free_ext(extPtr *hptr);
void free_words(wordPtr *hptr);
wordPtr update_dataimg_address(wordPtr *data_list);
void add_new_inst_node();
void add_new_data_node();
int get_from_previous_main_word(int field, wordMeta word);

/* Functions that handle errors */
int is_error();
void write_error(int line_num);


#endif
