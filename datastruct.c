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

/*function to get the instruction name as a string by index*/
char * getInstByCode(int idx) {
    if (idx >= 0 && idx < INST_SET_SIZE)
        return inst_prop[idx].inst;
        
    return NULL;
}

/*checks for the valid amount of operands for the given instruction*/
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

/*get the number of symbols in the symbols table*/
int getNumOfSymbols() {
    return num_of_symbols;
}


/**
 * The function `initSymbolsTbl` dynamically allocates memory for a symbol table in bulk sizes.
 * 
 * @return If the initialization of the symbols table is successful, it will return `RC_OK`. If there is an error due to an uninitialized
 * symbols table, it will return `RC_E_UNINITIALIZED_SYM_TBL`.
 */
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

/*get the symbols table*/
SymbolsTbl * getSymbolsTbl(void) {
    if (!symbols_tbl) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return NULL;
    }
    
    return symbols_tbl;
}

/**
 * The function `updateSymbolVal` updates the value of a symbol in a symbol table and adjusts the
 * instruction count accordingly.
 * 
 * @param symbol the symbol whose value needs to be updated.
 * @param symbol_typethe type of the symbol being updated. It is used to ensure that the symbol being updated matches the
 * specified type in the symbol table.
 * @param value the new value to be assigned to the symbol.
 * 
 * @return The function `updateSymbolVal` will return one of the following values:
 * - `RC_E_UNINITIALIZED_SYM_TBL` if the symbol table `symbols_tbl` is not initialized.
 * - `RC_OK` if the symbol with the given `symbol` name and `symbol_type` is found and updated
 * successfully.
 * - `RC_NOT_FOUND` if the symbol with the given `symbol` name was not found.
 */
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

/**
 * The function `addSymbolVal` adds a symbol with its type and value to a symbols table, handling
 * duplicate symbol errors.
 * 
 * @param symbol the symbol to be added to the symbol table.
 * @param symbol_type the type of the symbol being added.
 * @param value the numerical value associated with the symbol being added to the symbol table.
 * 
 * @return If the operation is successful, it returns `RC_OK`. If there is an error due to a duplicated symbol, it returns
 * `RC_E_DUPLICATED_SYMBOL`. If there is an error during the initialization of the symbols table, it
 * returns the error code from `initSymbolsTbl()`.
 */
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

/**
 * The function `symbolUsage` is responsible for managing the usage of symbols in a symbol table.
 * 
 * @param idx the index of a symbol in the `symbols_tbl` array.
 * @param ic the instruction counter value that is being checked against the existing symbol usage entries.
 * 
 * @return The function `symbolUsage` is returning the status code `RC_OK`.
 */
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

/**
 * The function `getSymbolTypeForARE` checks if a symbol exists in a symbol table and is not of type
 * DEFINEsym or EXTERNsym.
 * 
 * @param symbol Checks if it exists in the symbol table. If the symbol is found in the symbol table and its context is not `.define` or
 * `.extern`, it returns `RC_OK`.
 * 
 * @return If the symbol is found in the `symbols_tbl` and its `SymContext` is not `DEFINEsym` or `EXTERNsym`, it returns `RC_OK`. If the
 * `symbols_tbl` is not initialized, it returns `RC_E_UNINITIALIZED_SYM_TBL`. If the symbol is not found in the table, it returns NOT FOUND error.
 */
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

/**
 * The function `getSymbolVal` searches for a symbol in a table and returns its value along with
 * performing a usage check.
 * 
 * @param symbol the symbol whose value is being retrieved.
 * @param type used to specify the context in which the symbol should be searched for.
 * @param ic stands for instruction counter.
 * @param value the value corresponding to the symbol will be stored if the symbol is found in the symbol table.
 * 
 * @return If the symbols table is not initialized, the function will return the error code
 * `RC_E_UNINITIALIZED_SYM_TBL`. If the symbol with the specified label and context is found in the
 * symbols table, the function will set the value pointer to the value of the symbol and then call the
 * `symbolUsage` function with the index of the symbol and the given instruction counter `ic`. The
 * return value of symbolUsage is returned.
 */
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

/*used for debug purposes*/
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


/**
 * The function `getOpcodeTypeByOperand` determines the opcode type based on the operand type provided.
 * 
 * @param op_type represents different types of operands in an instruction. The possible values for `op_type` are `IMMEDIATE_VAL`, `IMMEDIATE_LABEL`, `DIRECT`,
 * `INDEX_NUM`, `INDEX_LABEL`, and `REGISTER`.
 * 
 * @return returns the opcode type.
 */
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

static Opcodes * opcodes_arr = NULL;
#define OPCODE_TABLE_BULK_SIZE 20
static int num_of_opcodes = 0;

/**
 * The function `initOpcodesTbl` dynamically allocates memory for an array of opcodes based on a bulk
 * size.
 * 
 * @return If the function is successful, it will return the value `RC_OK`. If there is an error due to uninitialized symbol table, it will
 * return the error code `RC_E_UNINITIALIZED_SYM_TBL`.
 */
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

/**
 * The function `addOpcode` updates the opcode value based on the given word type and number, and
 * increments the number of opcodes if specified.
 * 
 * @param wordtype represents the type of word (command line part) being processed.
 * @param num The `num` parameter in the `addOpcode` function represents the numerical value that needs
 * to be encoded into the opcode based on the `wordtype`.
 * @param inc_line Used to determine whether to increment the number of opcodes.
 * 
 * @return returns either `RC_OK` if the function execution was successful, or an error code indicating the reason for failure.
 */
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
        case ARE: /*ARE code to be added in bits 0 and 1*/
            cur_opcode->opcode |= (num & 0x3);
            break;
        
        case OPERAND_2_TYPE: /*for 2nd operand*/
            cur_opcode->opcode |= ((num & 0x3) << 2);
            break;
        
        case OPERAND_1_TYPE: /*for 1st operand*/
            cur_opcode->opcode |= ((num & 0x3) << 4);
            break;
        
        case INSTTYPE: /*for instructions*/
            cur_opcode->opcode |= ((num & 0xf) << 6);
            break;
        
        case VALUE: /*for numbers*/
            cur_opcode->opcode |= ((num & 0xfff) << 2);
            break;

        case VALUE_STR_DATA: /*used for strings and data types*/
            cur_opcode->opcode = (num & 0x3fff);
            break;

        case REGISTER_1: /*used for 1st register*/
            cur_opcode->opcode |= (num & 0x7) << 5;
            break;

        case REGISTER_2: /*used for 2nd register*/
            cur_opcode->opcode |= (num & 0x7) << 2;
            break;

        default:
            break;
    }

    return RC_OK;
}

/*get the opcodes array*/
Opcodes * getOpcodes(void) {
    if (!opcodes_arr) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_Opcode_TBL);
        return NULL;
    }

    return opcodes_arr;
}

/*used for debug purposes*/
static int getByteStr(char * byte_str, char c) {
    sprintf(byte_str, BYTE2BINSTR, BYTE2BIN(c));
    return RC_OK;
}

/*get the number of opcodes generated*/
int getNumOfOpcodes() {
    return num_of_opcodes;
}

/*used for debug purposes*/
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


/**
 * The function `my_strdup` dynamically allocates memory for a new string based on the input string
 * `src` with an optional delta value.
 * 
 * @param src a string that will be duplicated and its memory allocated.
 * @param delta can be used to specify an additional length to be allocated for the destination string. If `delta` is
 * greater than 0, it will be used as the length of the destination string plus one.
 * 
 * @return A dynamically allocated copy of the input string `src` with an additional `delta`
 * characters, if `delta` is greater than 0. Otherwise, a copy of the input string `src` is returned.
 */
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

/**
 * The function `isValidOperand` checks if a given operand type is valid for a specific instruction
 * type and operand index.
 * 
 * @param inst_type represents the type of instruction being checked.
 * @param op_type represents the type of operand being checked.
 * @param operand_index represents the index of the operand being checked within the instruction.
 * 
 * @return returns either `RC_OK` if the operand type is valid for the given instruction type and operand index,
 * or `RC_E_INVALID_OPERAND` if the operand type is not valid.
 */
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