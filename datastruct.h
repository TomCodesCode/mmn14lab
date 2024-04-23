#include "lib.h"

#define NUM_OF_OPERANDS 2

typedef struct Opcodes{
    unsigned int opcode : 14;
    char * symbol;
} Opcodes;

/*Define enums for instruction types and operand types*/
enum InstructionType {
    MOV, CMP, ADD, SUB, NOT, CLR, LEA, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, HLT
};

enum OperandType {
    IMMEDIATE_VAL, IMMEDIATE_LABEL, DIRECT, INDEX_NUM, INDEX_LABEL, REGISTER
};

#define OPCODE_OPERAND_TYPE_IMMEDIATE 0
#define OPCODE_OPERAND_TYPE_DIRECT    1
#define OPCODE_OPERAND_TYPE_INDEX     2
#define OPCODE_OPERAND_TYPE_REGISTER  3

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

enum cmd_type {
    INSTRUCTION, DIRECTIVE, DEFINE, EMPTY
};

typedef struct AST {
    char *label_occurrence;
    enum cmd_type cmd_type;
    int ic;
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

enum SymbolContext {
    CODEsym, STRINGsym, DATAsym, ENTRYsym, EXTERNsym, DEFINEsym
};

struct symbol_usage {
    int ic;
    struct symbol_usage * next;
};

typedef struct Symbols {
    char *label;
    enum SymbolContext SymContext;
    int value;
    struct symbol_usage * usage;
} SymbolsTbl;

enum WordType {
    ARE, OPERAND_1_TYPE, OPERAND_2_TYPE, INSTTYPE, VALUE, VALUE_STR, REGISTER_1, REGISTER_2
};

typedef struct Instructions {
    char *inst;
    int opcode;
    char *dest;
    char *src;
} Instructions;


#define BYTE2BINSTR "%c%c%c%c%c%c%c%c"
#define BYTE2BIN(c)  \
  ((c) & 0x80 ? '1' : '0'), ((c) & 0x40 ? '1' : '0'), ((c) & 0x20 ? '1' : '0'), ((c) & 0x10 ? '1' : '0'), \
  ((c) & 0x08 ? '1' : '0'), ((c) & 0x04 ? '1' : '0'), ((c) & 0x02 ? '1' : '0'), ((c) & 0x01 ? '1' : '0') 

