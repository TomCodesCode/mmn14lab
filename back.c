#include "lib.h"

static int createObFile(char *filename) {
    FILE * pFile = NULL;
    Opcodes * opcodes_table;
    int num_of_opcodes = getNumOfOpcodes();
    int val, i, j;
    char encr_code[ENCRYPTED_OPCODE_LEN+1];

    encr_code[ENCRYPTED_OPCODE_LEN] = 0;

    opcodes_table = getOpcodes();
    if (!opcodes_table || !num_of_opcodes) {
        PRINT_ERROR_MSG(RC_E_GET_OPCODE_TBL_FAILED);
        return RC_E_GET_OPCODE_TBL_FAILED;
    }

    pFile = fopen(filename, "w");
    if (!pFile) {
        PRINT_ERROR_MSG(RC_E_FAILED_TO_OPEN_FILE);
        return RC_E_FAILED_TO_OPEN_FILE;
    }

    fprintf(pFile, "%d %d\n", DC_START?(DC_START-IC_START):(PC-IC_START), DC_START?(PC-DC_START):0);
    
    for (i = 0; i < num_of_opcodes; i++) {
        fprintf(pFile, "[%04d] ", i + START_ADDRESS);
        val = opcodes_table[i].opcode;

        for (j = 0; j < ENCRYPTED_OPCODE_LEN; j++) {
            switch (val & 0x3) {
                case 3:
                    encr_code[ENCRYPTED_OPCODE_LEN - j - 1] = '!';
                    break;
                case 2:
                    encr_code[ENCRYPTED_OPCODE_LEN - j - 1] = '%';
                    break;
                case 1:
                    encr_code[ENCRYPTED_OPCODE_LEN - j - 1] = '#';
                    break;
                case 0:
                    encr_code[ENCRYPTED_OPCODE_LEN - j - 1] = '*';
                    break;
            }
            val >>= 2;
        }

        fprintf(pFile, "%s\n", encr_code);
    }

    if(pFile)
        fclose(pFile);

    return RC_OK;
}

static int createEntFile(char *filename) {
    FILE * pFile = NULL;
    int i, j;
    SymbolsTbl * sym_tbl = getSymbolsTbl();
    int num_of_symbols = getNumOfSymbols();

    for (i = 0; i < num_of_symbols; i++) {
        if (sym_tbl[i].SymContext != ENTRYsym)
            continue;

        if (!pFile) {
            pFile = fopen(filename, "w");
            if (!pFile) {
                PRINT_ERROR_MSG(RC_E_FAILED_TO_OPEN_FILE);
                PRINT_MSG_STR(filename);
                return RC_E_FAILED_TO_OPEN_FILE;
            }
        }

        for (j = 0; j < num_of_symbols; j++) {
            if (!strcmp(sym_tbl[i].label, sym_tbl[j].label) && sym_tbl[j].SymContext != ENTRYsym)
                fprintf(pFile, "%10s \t %04d \n", sym_tbl[j].label, sym_tbl[j].value);
        }
    }

    if(pFile)
        fclose(pFile);
        
    return RC_OK;
}

static int createExtFile(char *filename) {
    FILE * pFile = NULL;
    int i;
    struct symbol_usage * usage = NULL;
    SymbolsTbl * sym_tbl = getSymbolsTbl();
    int num_of_symbols = getNumOfSymbols();

    for (i = 0; i < num_of_symbols; i++) {
        if (sym_tbl[i].SymContext != EXTERNsym)
            continue;

        if (!pFile) {
            pFile = fopen(filename, "w");
            if (!pFile) {
                PRINT_ERROR_MSG(RC_E_FAILED_TO_OPEN_FILE);
                PRINT_MSG_STR(filename);
                return RC_E_FAILED_TO_OPEN_FILE;
            }
        }

        usage = sym_tbl[i].usage;
        while (usage) {
            if (usage->ic)
                fprintf(pFile, "%10s \t %04d \n", sym_tbl[i].label, usage->ic);
            usage = usage->next;
        }
    }

    if(pFile)
        fclose(pFile);
        
    return RC_OK;
}

int backendPass(char * filename) {
    char out_filename[MAX_FILENAME_LEN];
    int f_error =  FALSE;
    int rc;

    strcpy(out_filename, filename);
    strcat(out_filename, FN_OB_EXT);
    rc = createObFile(out_filename);
    if (rc != RC_OK) f_error = TRUE;

    strcpy(out_filename, filename);
    strcat(out_filename, FN_ENT_EXT);
    rc = createEntFile(out_filename);
    if (rc != RC_OK) f_error = TRUE;

    strcpy(out_filename, filename);
    strcat(out_filename, FN_EXT_EXT);
    rc = createExtFile(out_filename);
    if (rc != RC_OK) f_error = TRUE;

    if (f_error) {
        PRINT_ERROR_MSG(RC_E_BACKEND_FAILED);
        return RC_E_BACKEND_FAILED;
    }

    return RC_OK;
}