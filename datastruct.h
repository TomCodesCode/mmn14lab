#include "globals.h"
#include <stdio.h>

#define NUM_OF_OPERANDS 2

/*Define enums for instruction types and operand types*/
enum InstructionType {
    MOV, CMP, ADD, SUB, NOT, CLR, LEA, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, HLT
};

enum OperandType {
    IMMEDIATE, DIRECT, INDEX_NUM, INDEX_LABEL, REGISTER
};

struct Define {
    char *label;
    int number;
};


/*Define structures for instruction operands*/
struct Operand {
    enum OperandType type;
    union {
        int immediate;
        char *label;
        int reg;
        struct {
            char *label1;
            union {
                int number;
                char *label2;
            } index_select;
        } index_op;
    } operand_select;
};

/*Define structures for directives*/
enum DirectiveType {
    STRING, ENTRY, EXTERN, DATA
};

struct DataDirective {
    char *label;
    union {
        int number;
        char *label;
    } data_options;
};

struct String {
    char *label;
    char *string;
};


struct Directive {
    enum DirectiveType type;
    union {
        struct String string; /*for .string*/
        char *label; /*for .entry and .extern*/
        struct DataDirective data [50]; /*for .data. can  hold up to 30 objects.*/
    } directive_options;
};

typedef struct AST {
    char *errors[20];
    char *label_occurrence;
    enum {
        INSTRUCTION, DIRECTIVE, DEFINE, EMPTY
    } cmd_type;

    union {
        struct Define define;

        struct {
            enum InstructionType inst_type;
            struct Operand operands[NUM_OF_OPERANDS];
        } instruction;

        struct Directive directive;
    } command;

    struct AST *next;

} AST;

typedef struct Labels {
    char label_name [MAX_TOKEN_LENGTH];
    int label_adress;
}Labels;

typedef struct Instructions {
    char *inst;
    int opcode;
    char *src;
    char *dest;
} Instructions;

Instructions inst_prop[INST_SET_SIZE] = {{"mov", 0, "0123", "123"}, {"cmp", 1, "0123", "0123"}, {"add", 2, "0123", "123"}, {"sub", 3,"0123", "123"},
                            {"not", 4, "-", "123"}, {"clr", 5, "-", "123"}, {"lea", 6, "12", "123"}, {"inc", 7, "-", "123"},
                            {"dec", 8, "-", "123"}, {"jmp", 9, "-", "13"}, {"bne", 10, "-", "13"}, {"red", 11, "-", "123"},
                            {"prn", 12, "-", "0123"}, {"jsr", 13, "-", "13"}, {"rts", 14, "-", "-"}, {"hlt", 15, "-", "-"}};
