#include "datastruct.h"
#include "errors.h"

Instructions inst_prop[INST_SET_SIZE] = {
    {"mov",	0,	"123",	"0123"},
    {"cmp",	1,	"0123",	"0123"},
    {"add",	2,	"123",	"0123"},
    {"sub",	3,	"123",	"0123"},
    {"not",	4,	"123",	 NULL},
    {"clr",	5,	"123",	 NULL},
    {"lea",	6,	"123",	"12"},
    {"inc",	7,	"123",	 NULL},
    {"dec",	8,	"123",	 NULL},
    {"jmp",	9,	"13",	 NULL},
    {"bne",	10,	"13",	 NULL},
    {"red",	11,	"123",	 NULL},
    {"prn",	12,	"0123",	 NULL},
    {"jsr",	13,	"13",	 NULL},
    {"rts",	14,	 NULL,	 NULL},
    {"hlt",	15,	 NULL,	 NULL}
};

char * getInstByIdx(int idx) {
    if (idx >= 0 && idx < INST_SET_SIZE)
        return inst_prop[idx].inst;
        
    return NULL;
}

int numValidInstOperands(int inst) {
    if (inst_prop[inst].dest && inst_prop[inst].src)
        return 2;

    if (inst_prop[inst].dest)
        return 1;

    return 0;
}

int toBinary(int num, char *opcode_str, int start, int end){
    int i = start;
    int num_cpy = num;
    while (num > 0 && i < end) {
        opcode_str[i++] = (num & 1) + '0';
        num >>= 1;
    }
    
    if (num_cpy < 0){
        i = 0;
        while (i < TOTAL_BITS){
            if (opcode_str[i] == '0') opcode_str[i] == '1';
            if (opcode_str[i] == '1') opcode_str[i] == '0';
            i++;
        }
        opcode_str[i] = '\0';
    }
    return RC_OK;
}

char *calcOpcode(AST *node){}

int getOpcode(AST *node){
    int i = 0;
    int rc;
    int operand_type;
    int operands_num;
    char *opcode_str = (char *)calloc(TOTAL_BITS, sizeof(char));
    char *operator_str;

    switch (node->cmd_type){
        case INSTRUCTION:
            rc = toBinary(node->command.instruction.inst_type, opcode_str, ARE_BITS + SRC_BITS + DST_BITS, ARE_BITS + SRC_BITS + DST_BITS + OPCODE_BITS); /*insert opcode bits (6-9)*/
            operands_num = numValidInstOperands(node->command.instruction.inst_type);
            if (operands_num > 0){
                operand_type = node->command.instruction.operands[0].type;
                if (operand_type == IMMEDIATE){
                    rc = toBinary(0, opcode_str, ARE_BITS + DST_BITS, ARE_BITS + DST_BITS + SRC_BITS); /*insert src bits (4-5)*/
                    if (rc != RC_OK) return RC_E_RUNTIME_FAIL;
                }
                else if (operand_type == DIRECT){
                    rc = toBinary(1, opcode_str, ARE_BITS + DST_BITS, ARE_BITS + DST_BITS + SRC_BITS); /*insert src bits (4-5)*/
                    if (rc != RC_OK) return RC_E_RUNTIME_FAIL;
                }
                else if (operand_type == INDEX_NUM || operand_type == INDEX_LABEL){
                    rc = toBinary(2, opcode_str, ARE_BITS + DST_BITS, ARE_BITS + DST_BITS + SRC_BITS); /*insert src bits (4-5)*/
                    if (rc != RC_OK) return RC_E_RUNTIME_FAIL;
                    if (operands_num > 1){
                        operand_type = node->command.instruction.operands[1].type;
                        if (operand_type == INDEX_NUM || operand_type == INDEX_LABEL){
                            rc = toBinary(2, opcode_str, ARE_BITS, ARE_BITS + DST_BITS); /*insert dst bits (3-4)*/
                            if (rc != RC_OK) return RC_E_RUNTIME_FAIL;
                            operands_num++;
                        }
                    }
                    operands_num++;
                }
                else if (operand_type == REGISTER){
                    rc = toBinary(3, opcode_str, ARE_BITS + DST_BITS, ARE_BITS + DST_BITS + SRC_BITS); /*insert src bits (4-5)*/
                    if (rc != RC_OK) return RC_E_RUNTIME_FAIL;
                }else{
                    return RC_E_INVALID_OPERAND;
                }
            }
            free(opcode_str);
            for (; i < operands_num; i++){
            }
            break;
        case DIRECTIVE:
            break;
        case DEFINE:
            break;
        default:
            break;
    }
    return RC_OK;
}