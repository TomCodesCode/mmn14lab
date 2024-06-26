#include "lib.h"

#define NUM_OF_OPERANDS 2

/*used to partition lines into tokens*/
#define WILDCARD_DELIMITERS " \t\n\r"
#define TOKEN_DELIMITERS " =,\t\n\r"

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

/*Define enum for type data number(label by define or number)*/
enum DataTypeEnum{
    NUMBER_DATA, LABEL_DATA
};

struct DataDirective {
    char *label;
    enum DataTypeEnum type;
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
    int data_objects;
    union {
        struct String string; /*for .string*/
        char *label; /*for .entry and .extern*/
        struct DataDirective data [MAX_DATA_OBJECTS]; /*for .data. can  hold up to 30 objects.*/
    } directive_options;
};

enum cmd_type {
    INSTRUCTION, DIRECTIVE, DEFINE, EMPTY
};

/*main data struct- Abstract Syntax Tree.*/
typedef struct AST {
    char *label_occurrence;
    enum cmd_type cmd_type;
    int ic;
    int dc;
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

/*used to differentiate different types of symbols*/
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

/*used to generate opcodes*/
enum WordType {
    ARE, OPERAND_1_TYPE, OPERAND_2_TYPE, INSTTYPE, VALUE, VALUE_STR_DATA, REGISTER_1, REGISTER_2
};

/*instruction types and their respective valid addressing types*/
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

