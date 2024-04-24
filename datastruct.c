#include "lib.h"

/* dest and src are swapped (relative to the booklet table) for the sake of convenience*/
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
    if (!symbols_tbl) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return NULL;
    }
    
    return symbols_tbl;
}

int updateSymbolVal(char * symbol, int symbol_type, int value) {
    int i;
    struct symbol_usage * usage;

    if (!symbols_tbl) 
        return RC_E_UNINITIALIZED_SYM_TBL;

    for (i = 0; i < num_of_symbols; i++){
        if(!strcmp(symbol, symbols_tbl[i].label) && symbol_type == symbols_tbl[i].SymContext) {
            usage = symbols_tbl[i].usage;
            while (usage) {
                usage->ic += (value - symbols_tbl[i].value);
                usage = usage->next;
            }
            symbols_tbl[i].value = value;
            return RC_OK;
        }
    }
    return RC_NOT_FOUND;
}

int addSymbolVal(char * symbol, int symbol_type, int value) {
    int val_tmp;
    int rc;

    if (symbol[strlen(symbol)-1] == ':')
        symbol[strlen(symbol)-1] = 0;

    if (RC_OK == getSymbolVal(symbol, symbol_type, 0, &val_tmp)) { /*added symbol type as a parameter*/
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

static int symbolUsage(int idx, int ic)
{
    struct symbol_usage * usage;

    if (symbols_tbl[idx].usage == NULL) {
        symbols_tbl[idx].usage = calloc(1, sizeof(struct symbol_usage));
        if (symbols_tbl[idx].usage == NULL) {
            PRINT_ERROR_MSG(RC_E_ALLOC_FAILED);
            exit (RC_E_ALLOC_FAILED);
        }
    }
    usage = symbols_tbl[idx].usage;

    while (usage->next) {
        if (usage->ic == ic)
            return RC_OK;
        usage = usage->next;
    }
    
    usage->next = calloc(1, sizeof(struct symbol_usage));
    usage = usage->next;

    if (usage == NULL) {
        PRINT_ERROR_MSG(RC_E_ALLOC_FAILED);
        exit (RC_E_ALLOC_FAILED);
    }

    usage->ic = ic;
    return RC_OK;
}

int getSymbolTypeForARE(const char * symbol){
    int i;

    if (!symbols_tbl) 
        return RC_E_UNINITIALIZED_SYM_TBL;

    for (i = 0; i < num_of_symbols; i++){
        if(!strcmp(symbol, symbols_tbl[i].label) && symbols_tbl[i].SymContext  != DEFINEsym && symbols_tbl[i].SymContext != EXTERNsym){
            return RC_OK;
        }
    }
    return RC_NOT_FOUND;
}

int getSymbolVal(const char * symbol, enum SymbolContext type, int ic, int * value) {
    int i;
    int rc;

    if (!symbols_tbl) {
        return RC_E_UNINITIALIZED_SYM_TBL;
    }

    for (i = 0; i < num_of_symbols; i++) {
        if (!strcmp(symbol, symbols_tbl[i].label) && type == symbols_tbl[i].SymContext) {
            *value = symbols_tbl[i].value;

            rc = symbolUsage(i, ic);

            return rc;
        }
    }

    return RC_NOT_FOUND;
}

int dumpSymbolTbl(void) {
    int i;
    struct symbol_usage * usage;

    printf("===DUMP SYMBOL TABLE ===\n");

    if (!symbols_tbl) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return RC_E_UNINITIALIZED_SYM_TBL;
    }

    for (i = 0; i < num_of_symbols; i++) {
        printf("[%d] Symbol: %s Type: %d Value: %d ", i, symbols_tbl[i].label, symbols_tbl[i].SymContext, symbols_tbl[i].value);
        printf("Usage: ");
        usage = symbols_tbl[i].usage;

        while (usage) {
            if (usage->ic)
                printf("%d ", usage->ic);
            usage = usage->next;
        }
        printf("\n");
    }

    return RC_OK;
}


int getOpcodeTypeByOperand(enum OperandType op_type) {
    switch (op_type) {
        case IMMEDIATE_VAL:
        case IMMEDIATE_LABEL:
            return OPCODE_OPERAND_TYPE_IMMEDIATE;
        case DIRECT:
            return OPCODE_OPERAND_TYPE_DIRECT;
        case INDEX_NUM:
        case INDEX_LABEL:
            return OPCODE_OPERAND_TYPE_INDEX;
        case REGISTER:
            return OPCODE_OPERAND_TYPE_REGISTER;
        default:
            break;
    }
    PRINT_ERROR_MSG(RC_E_INVALID_OPERAND);
    return RC_E_INVALID_OPERAND;
}

/*
 * Opcode generation
 */
static Opcodes * opcodes_arr = NULL;
#define OPCODE_TABLE_BULK_SIZE 20
static int num_of_opcodes = 0;

static int initOpcodesTbl(void) {
    Opcodes * opcodes_arr_tmp = NULL;
    if (!(num_of_opcodes % OPCODE_TABLE_BULK_SIZE)) {
        if (!opcodes_arr)
            opcodes_arr_tmp = (Opcodes *)calloc(sizeof(Opcodes), OPCODE_TABLE_BULK_SIZE);
        else
            opcodes_arr_tmp = (Opcodes *)realloc(opcodes_arr, (num_of_opcodes + OPCODE_TABLE_BULK_SIZE)*sizeof(Opcodes));

        opcodes_arr = opcodes_arr_tmp;
    }
    if (!opcodes_arr) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return RC_E_UNINITIALIZED_SYM_TBL;
    }
    return RC_OK;
}

int addOpcode(int wordtype, int num, enum Bool inc_line){
    int rc;
    Opcodes * cur_opcode;
    int opcode_idx;

    rc = initOpcodesTbl();
    if (rc != RC_OK)
        return rc;
    
    if (inc_line)
        num_of_opcodes++;

    if (!num_of_opcodes) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return RC_E_UNINITIALIZED_SYM_TBL;
    }

    opcode_idx = num_of_opcodes - 1;

    cur_opcode = &(opcodes_arr[opcode_idx]);

    switch (wordtype){
        case ARE:
            cur_opcode->opcode |= (num & 0x3);
            break;
        
        case OPERAND_2_TYPE:
            cur_opcode->opcode |= ((num & 0x3) << 2);
            break;
        
        case OPERAND_1_TYPE:
            cur_opcode->opcode |= ((num & 0x3) << 4);
            break;
        
        case INSTTYPE:
            cur_opcode->opcode |= ((num & 0xf) << 6);
            break;
        
        case VALUE:
            cur_opcode->opcode |= ((num & 0xfff) << 2);
            break;

        case VALUE_STR_DATA:
            cur_opcode->opcode = (num & 0x3fff);
            break;

        case REGISTER_1:
            cur_opcode->opcode |= (num & 0x7) << 5;
            break;

        case REGISTER_2:
            cur_opcode->opcode |= (num & 0x7) << 2;
            break;

        default:
            break;
    }

    return RC_OK;
}

Opcodes * getOpcodes(void) {
    if (!opcodes_arr) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_Opcode_TBL);
        return NULL;
    }

    return opcodes_arr;
}

static int getByteStr(char * byte_str, char c) {
    sprintf(byte_str, BYTE2BINSTR, BYTE2BIN(c));
    return RC_OK;
}

int dumpOpcodesTbl(void) {
    int i;
    char byte_str[9];

    for (i = 0; i < num_of_opcodes; i++) {
        printf("[%04d] ", i + 100);
        getByteStr(byte_str, (opcodes_arr[i].opcode>>8)&0xff);
        byte_str[0] = ' ';
        byte_str[1] = ' ';
        printf("%s", byte_str);
        getByteStr(byte_str, opcodes_arr[i].opcode&0xff);
        printf("%s", byte_str);
        printf("\n");
    }

    return RC_OK;
}


char *my_strdup(const char *src, int delta) {
    int len = (delta > 0) ? (delta + 1) : strlen(src) + 1;
    char *dst = (char *)malloc(len);

    if (dst == NULL) {
        PRINT_ERROR_MSG(RC_E_ALLOC_FAILED);
        exit(RC_E_ALLOC_FAILED);
    }

    memcpy(dst, src, len - 1);
    dst[len - 1] = '\0';

    return dst;
}

int isValidOperand(enum InstructionType inst_type, enum OperandType op_type, int operand_index){

    int actual_optype;
    char *inst_option;
    
    if (op_type == IMMEDIATE_VAL || op_type == IMMEDIATE_LABEL) actual_optype = 0;
    if (op_type == DIRECT) actual_optype = 1;
    if (op_type == INDEX_NUM || op_type == INDEX_LABEL) actual_optype = 2;
    if (op_type == REGISTER) actual_optype = 3;

    if (operand_index == 0 && inst_prop[inst_type].dest){
        inst_option = inst_prop[inst_type].dest;
        while (inst_option) {
            if (*inst_option == actual_optype + 48)
                return RC_OK;
            inst_option++;
        }
    }

    else if (operand_index == 1 && inst_prop[inst_type].src){
        inst_option = inst_prop[inst_type].src;
        while (inst_option) {
            if (*inst_option == actual_optype + 48)
                return RC_OK;
            inst_option++;
        }
    }
    
    return RC_E_INVALID_OPERAND;
}