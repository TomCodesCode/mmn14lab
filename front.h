#include "globals.h"
typedef char * t_commandLine[MAX_COMMAND_SIZE];



/*

typedef struct AST {
    enum {
        ast_instruction,
        ast_directive,
        ast_define,
        ast_empty
    }ast_type;

    union {
        struct {
            const char *label;
            int number;
        }define;
        struct{
            enum {
                ast_extern,
                ast_entry,
                ast_string,
                ast_data
            } directive_type;
            union
            {
                char *label;
                char *string;
                struct {
                    enum {
                        data_label,
                        daa_number
                    }data_type;
                    union {
                        char *label;
                        int number;
                    }data_options;
                };

            }directive_options;

        }directive;


        struct {
            enum {
                ast_mov,
                ast_cmp,
                ast_add,
                ast_sub,
                ast_not,
                ast_clr,
                ast_lea,
                ast_inc,
                ast_dec,
                ast_jmp,
                ast_bne,
                ast_red,
                ast_prn,
                ast_jsr,
                ast_rts,
                ast_hlt
            }instruction_type;
            struct {
                enum {
                    ast_immed,
                    ast_label,
                    ast_index_const,
                    ast_index_label,
                    ast_register
                }operand_type;
                union {
                    int immed;
                    char *label;
                    int reg;
                    struct {
                        char *label;
                        union {
                            int number;
                            char *label;
                        }index_options;
                    };

                }operand_options;

            }operands[2];

        }instruction;

    }ast_options;

}AST;
*/

/*
#include <stdio.h>
#include <stdlib.h>

// Define the types of AST nodes
enum NodeType {
    INSTRUCTION_NODE,
    DIRECTIVE_NODE
};

// Define the types of instructions
enum InstructionType {
    MOV,
    CMP,
    ADD,
    SUB,
    NOT,
    CLR,
    LEA,
    INC,
    DEC,
    JMP,
    BNE,
    RED,
    PRN,
    JSR,
    RTS,
    HLT
};

// Define the types of directives
enum DirectiveType {
    EXTERN,
    ENTRY,
    STRING,
    DATA
};

// Define the AST node structure
typedef struct ASTNode {
    enum NodeType type;
    union {
        struct {
            enum InstructionType instruction;
            // Additional instruction-specific data can be added here
        } instructionNode;
        struct {
            enum DirectiveType directive;
            // Additional directive-specific data can be added here
        } directiveNode;
    } data;
    struct ASTNode *next; // Pointer to the next node in the AST
} ASTNode;

// Function to create a new instruction node
ASTNode *createInstructionNode(enum InstructionType instructionType) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (node != NULL) {
        node->type = INSTRUCTION_NODE;
        node->data.instructionNode.instruction = instructionType;
        node->next = NULL;
    }
    return node;
}

// Function to create a new directive node
ASTNode *createDirectiveNode(enum DirectiveType directiveType) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (node != NULL) {
        node->type = DIRECTIVE_NODE;
        node->data.directiveNode.directive = directiveType;
        node->next = NULL;
    }
    return node;
}

int main() {
    // Example usage
    ASTNode *movInstruction = createInstructionNode(MOV);
    ASTNode *externDirective = createDirectiveNode(EXTERN);
    // Add more nodes as needed
    return 0;
}
*/

/*
typedef struct AST {
    char errors[100];
    enum {
        instruction_type,
        directive_type,
        define_type,
        empty_type
    }line_type;

    union {
        typedef struct instruction {
            enum {
                MOV,
                CMP,
                ADD,
                SUB,
                NOT,
                CLR,
                LEA,
                INC,
                DEC,
                JMP,
                BNE,
                RED,
                PRN,
                JSR,
                RTS,
                HLT
            }inst_type;
            struct {
                enum {
                    immed_0,
                    direct_1,
                    index_2_num,
                    index_2_reg,
                    index_2_label,
                    reg_3
                }op_type;
                union {
                    int immed;
                    char *label;
                    int reg;
                    struct {
                        char *label1;
                        union {
                            int number;
                            char * label2;
                        } index_select;

                    }index_op;

                }operand_select;

            }operands [2];

        }instruction;

        typedef struct directive {
            enum {
                string_type,
                entry_type,
                extern_type,
                data_type
            }dir_type;
            union {
                char *string; //for string type
                char *label; //for enrty and extern
                typedef struct data {
                    char *label;
                    union {
                        int reg;
                        char *label;
                    };
                }data;


            };


        }directive;


    }commands;
}AST;*/
