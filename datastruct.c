#include "lib.h"


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
/*
static SymbolsTbl * getSymbolsTbl(void) {
    if (!symbols_tbl) 
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
    
    return symbols_tbl;
}*/

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
