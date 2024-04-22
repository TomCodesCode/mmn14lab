#include "lib.h"
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

static SymbolsTbl * symbols_tbl = NULL;
static int num_of_symbols = 0;
#define SYM_TABLE_BULK_SIZE 20

static int initSymbolsTbl(void) {
    SymbolsTbl * symbols_tbl_tmp = NULL;
    if (!(num_of_symbols % SYM_TABLE_BULK_SIZE)) {
        if (!symbols_tbl)
            symbols_tbl_tmp = (SymbolsTbl *)calloc(sizeof(SymbolsTbl), SYM_TABLE_BULK_SIZE);
        else
            symbols_tbl_tmp = (SymbolsTbl *)realloc(symbols_tbl, (num_of_symbols + SYM_TABLE_BULK_SIZE)*sizeof(SymbolsTbl));

        symbols_tbl = symbols_tbl_tmp;
    }
    if (!symbols_tbl) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return RC_E_UNINITIALIZED_SYM_TBL;
    }
    return RC_OK;
}

SymbolsTbl * getSymbolsTbl(void) {
    if (!symbols_tbl) 
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
    
    return symbols_tbl;
}

int addSymbolVal(char * symbol, int symbol_type, int value) {
    int val_tmp;
    int rc;

    if (RC_OK == getSymbolVal(symbol, &val_tmp)) {
        PRINT_ERROR_MSG(RC_E_DUPLICATED_SYMBOL);
        printf("Symbol %s already exists\n", symbol);
        return RC_E_DUPLICATED_SYMBOL;
    }

    rc = initSymbolsTbl();
    if (rc != RC_OK)
        return rc;
    
    symbols_tbl[num_of_symbols].label = symbol;
    symbols_tbl[num_of_symbols].SymContext = symbol_type;
    symbols_tbl[num_of_symbols].value = value;

    num_of_symbols++;
    return RC_OK;
}

int getSymbolVal(const char * symbol, int * value) {
    int i;
    if (!symbols_tbl) {
        return RC_E_UNINITIALIZED_SYM_TBL;
    }

    for (i = 0; i < num_of_symbols; i++) {
        if (!strcmp(symbol, symbols_tbl[i].label)) {
            *value = symbols_tbl[i].value;
            return RC_OK;
        }
    }

    return RC_NOT_FOUND;
}

int dumpSymbolTbl(void) {
    int i;
    if (!symbols_tbl) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return RC_E_UNINITIALIZED_SYM_TBL;
    }

    for (i = 0; i < num_of_symbols; i++) {
        printf("[%d] Symbol: %s Type: %d Value: %d\n", i, symbols_tbl[i].label, symbols_tbl[i].SymContext, symbols_tbl[i].value);
    }

    return RC_OK;
}

#if 0

int calcOpcodePart(Opcodes *opcode_node, int wordtype, int num, int in_line_part){
    int neg_bool = FALSE;
    Opcodes *opcode_node_cpy = opcode_node;

    /*ast->opcode[in_line_part].opcode = 0;*/
    switch (wordtype){
        case ARE:
            ast->opcode[in_line_part].opcode |= (num & 0x3);
            break;
        
        case OPERAND_2_TYPE:
            ast->opcode[in_line_part].opcode |= ((num & 0x3) << 2);
            break;
        
        case OPERAND_1_TYPE:
            ast->opcode[in_line_part].opcode |= ((num & 0x3) << 4);
            break;
        
        case INSTTYPE:
            ast->opcode[in_line_part].opcode |= ((num & 0xf) << 6);
            break;
        
        case VALUE:
            if (num < 0){
                neg_bool = TRUE;
                num = (-1) * num;
            ast->opcode[in_line_part].opcode |= (num & 0xf) << 2;
            if (neg_bool){
                num = ~num;
                num++;
            }
            break;
            }

        default:
            break;
    }
    ast->opcode[in_line_part].opcode |= opcode_cpy.opcode;

    return RC_OK;
}

int opcodePerOperand(AST *ast, int num, int operand_idx){
    int rc;
    if (operand_idx == 0)
        rc = calcOpcodePart(ast, OPERAND_1_TYPE, num, operand_idx + 1);
    else if (operand_idx == 1)
        rc = calcOpcodePart(ast, OPERAND_2_TYPE, num, operand_idx + 1);
    else
        rc = RC_E_FAILED_RETRIEVE_OPERANDS;
    return rc;
}

#endif
