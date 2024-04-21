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
    int IC;
    int DC;
    union {
        int number;
        char *label;
    } data_options;
};

struct String {
    char *label;
    char *string;
    int IC;
    int DC;
};


struct Directive {
    enum DirectiveType type;
    union {
        struct String string; /*for .string*/
        char *label; /*for .entry and .extern*/
        struct DataDirective data [50]; /*for .data. can  hold up to 30 objects.*/
    } directive_options;
};

typedef struct Opcode {
    unsigned short op : 14;
}Opcode;

typedef struct AST {
    char *label_occurrence;
    Opcode opcode[MAX_COMMAND_SIZE];
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


enum SymbolContext{
    OCCURRENCEsym, STRINGsym, DATAsym, ENTRYsym, EXTERNsym, DEFINEsym
};

typedef struct Symbols{ /*MODIFY TOMORROW*/
    char *label;
    int IC;
    enum SymbolContext SymContext;
    AST *sym_to_ast;

}Symbols;

typedef struct Instructions {
    char *inst;
    int opcode;
    char *dest;
    char *src;
} Instructions;

int numValidInstOperands(int inst);
char * getInstByIdx(int idx);

