#ifndef ASSEMBLER_H

#define ASSEMBLER_H

/** Defaults **/
#define MEMORY_START 100
#define NO_ERROR 0
#define ERROR 1
#define NOT_FOUND -1
#define EXTERNAL_DEFAULT_ADDRESS 0

/** Limits **/
#define LINE_LENGTH 80 /* maximum chars per line */
#define MINIMUM_LABEL_LENGTH_WITH_COLON 2
#define MINIMUM_LABEL_LENGTH_WITHOUT_COLON 1
#define LABEL_LENGTH 32 /* maximum chars per label */

#define MAX_FILE_EXTENSION 5 /* maximum number of characters in file extension (.ext/.ent) plus \0 */

#define REGISTER_LENGTH 2 /* a register's name contains 2 characters */

/* Other Constants */
#define NUM_DIRECTIVES 4 /* number of existing directives*/
#define NUM_COMMANDS 16 /* number of existing commands */
#define WORD_LENGTH 24

/*** Enums ***/

/* Define boolean in C */
typedef enum {FALSE, TRUE} boolean;

/* Directives types */
enum directives {DATA, STRING, ENTRY, EXTERN, UNKNOWN_DIRECTIVE};

/* Enum of commands ordered by their opcode */
enum commands {MOV, CMP, ADD, SUB, LEA, CLR, NOT, INC, DEC, JMP, BNE, JSR, RED, PRN, RTS, STOP, UNKNOWN_COMMAND};

/* Addressing methods ordered by their code */
enum methods {IMMEDIATE_ADDRESS, DIRECT_ADDRESS, RELATIVE_ADDRESS, REGISTER_ADDRESS, UNKNOWN_ADDRESS};

/* Struct fields */
enum struct_fields {ADDRESS, OPCODE, SOURCE_ADDRESSING, SOURCE_REGISTER, DEST_ADDRESSING, DEST_REGISTER, FUNCT};

enum word_types {MAIN_WORD = 1, BIT21_WORD, BIT24_WORD};

/* Types of files that indicate what is the desirable file extension */
enum file_types {FILE_INPUT, FILE_OBJECT, FILE_ENTRY, FILE_EXTERN};

/* Word address type*/
enum address_types {INST_IMG, DATA_IMG};

/* When we need to specify if label should contain a colon or not */
enum {NO_COLON, COLON};

enum errors {
    SYNTAX_ERR = 1, LABEL_ALREADY_EXISTS, LABEL_TOO_LONG, LABEL_INVALID_FIRST_CHAR, LABEL_ONLY_ALPHANUMERIC,
    LABEL_CANT_BE_COMMAND, LABEL_ONLY, LABEL_CANT_BE_REGISTER,
    DIRECTIVE_NO_PARAMS, DIRECTIVE_INVALID_NUM_PARAMS, DATA_COMMAS_IN_A_ROW, DATA_EXPECTED_NUM,
    DATA_EXPECTED_COMMA_AFTER_NUM, DATA_UNEXPECTED_COMMA,
    STRING_TOO_MANY_OPERANDS, STRING_OPERAND_NOT_VALID, EXPECTED_COMMA_BETWEEN_OPERANDS,
    EXTERN_NO_LABEL, EXTERN_INVALID_LABEL, EXTERN_TOO_MANY_OPERANDS,
    COMMAND_NOT_FOUND, COMMAND_UNEXPECTED_CHAR, COMMAND_TOO_MANY_OPERANDS,
    COMMAND_INVALID_METHOD, COMMAND_INVALID_NUMBER_OF_OPERANDS, COMMAND_INVALID_OPERANDS_METHODS,
    ENTRY_LABEL_DOES_NOT_EXIST, ENTRY_CANT_BE_EXTERN, COMMAND_LABEL_DOES_NOT_EXIST,
    CANNOT_OPEN_FILE
};


#endif

