#ifndef MMN14_PROTOTYPES_H
#define MMN14_PROTOTYPES_H

#include <stdio.h>
#include "structs.h"


void first_pass(FILE *fp);
void second_run(FILE *fp, char *filename);

/* First Run Functions */
void read_line(char *line);
int handle_directive(int type, char *line);
int handle_command (int type, char *line);
int handle_string_directive(char *line);
int handle_data_directive(char *line);
void write_num_to_data(int num);
void write_string_to_data(char *str);
int detect_method(char * operand);
boolean command_accept_methods(int type, int first_method, int second_method);
boolean command_accept_num_operands(int type, boolean first, boolean second);
void encode_word(int type, int is_first, int is_second, int first_method, int second_method, char *first_op, char *second_op);
int find_opcode(int type);
int find_funct(int type);
int register_value(char *operand);
int handle_extern_directive(char *line);
boolean is_label(char *token, int colon);

/* Second Run Functions */
void read_line_2nd(char *line);
void check_operands_exist(int type, boolean *is_src, boolean *is_dest);
unsigned int extract_word_info(int field, wordMeta word);
int command_manipulation_2nd(int type, char *line);
void proceed_to_blank_word(wordPtr *word);
void proceed_to_main_word(wordPtr *word);
void encode_additional_words(char *operand, int method);
void add_label_to_extern(unsigned int address, char *label_name);
void write_output_entry(FILE *fp);
void write_output_extern(FILE *fp);
int write_output_files(char *original);
void write_output_ob(FILE *fp);

#endif
