#ifndef MMN14_EXTERNS_H
#define MMN14_EXTERNS_H

#include "structs.h"
#include "assembler.h"

/* Global vars */
extern int ic, dc;
extern int err;
extern boolean was_error;
extern boolean entry_existence, extern_existence;
extern const char *commands[];
extern const char *directives[];

/* Data Structures */
extern wordPtr word_list;
extern wordPtr head_word;
extern labelPtr symbols_table;
extern wordPtr data_word_list;
extern wordPtr head_data_word;

#endif
