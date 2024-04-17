#include "globals.h"
#include <stdio.h>

// Define enums for instruction types and operand types
enum InstructionType {
    MOV, CMP, ADD, SUB, NOT, CLR, LEA, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, HLT
};

enum OperandType {
    IMMEDIATE, DIRECT, INDEX_NUM, INDEX_REG, INDEX_LABEL, REGISTER
};

struct Define {
    char *label;
    int number;
};


// Define structures for instruction operands
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
// Define structures for directives
enum DirectiveType {
    STRING, ENTRY, EXTERN, DATA
};

struct DataDirective {
    char *label;
    union {
        int reg;
        char *label;
    } data_options;
};

struct Directive {
    enum DirectiveType type;
    union {
        char *string;
        char label[MAX_LABEL_LENGTH];
        struct DataDirective data;
    } directive_options;
};

// Define the AST structure
typedef struct AST {
    char *errors[20];
    enum {
        INSTRUCTION, DIRECTIVE, DEFINE, EMPTY
    } line_type;

    union {
        struct Define define;
        struct {
            enum InstructionType inst_type;
            struct Operand operands[2];
        } instruction;

        struct Directive directive;
    } commands;

    AST *next;
    
} AST;

typedef struct Labels {
    char label_name [MAX_LABEL_LENGTH];
    int label_adress;
}Labels;

int main() {
    // Example usage
    AST ast;

    // Initialize and use the AST structure
    ast.line_type = INSTRUCTION;
    ast.commands.instruction.inst_type = MOV;
    ast.commands.instruction.operands[0].type = IMMEDIATE;
    ast.commands.instruction.operands[0].operand_select.immediate = 10;

    printf("AST created successfully.\n");

    return 0;
}
