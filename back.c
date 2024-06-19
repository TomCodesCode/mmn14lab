#include "lib.h"

/**
 * The function `createObFile` writes encrypted opcode data to a file based on the opcodes table.
 * 
 * @param filename a string representing the name of the file to be created.
 * 
 * @return returns an integer value, which is either an error code indicating the reason for failure or `RC_OK`.
 */
static int createObFile(char *filename) {
    FILE * pFile = NULL;
    Opcodes * opcodes_table;
    int num_of_opcodes = getNumOfOpcodes();
    int val, i, j;
    int addr;
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

    /*printing the amount of code lines and then the amount of data lines.*/
    fprintf(pFile, "%d %d\n", DC_START?(DC_START-IC_START):(PC-IC_START), DC_START?(PC-DC_START):0);
    addr = START_ADDRESS;
    for (i = 0; i < num_of_opcodes; i++, addr++) {
        if (addr >= MAX_RAM_SIZE) {
            PRINT_ERROR_MSG(RC_E_OUT_OF_MEMORY);
            fclose(pFile);
            return RC_E_OUT_OF_MEMORY;
        }
        fprintf(pFile, "[%04d] ", addr);
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
            val >>= 2; /*in each itteration, move 2 bits to the right*/
        }

        fprintf(pFile, "%s\n", encr_code);
    }

    if(pFile)
        fclose(pFile);

    return RC_OK;
}

/**
 * The function `createEntFile` iterates through symbols, writes entries to a file, and returns a
 * status code.
 * 
 * @param filename represents the name of the file to be created. This file will contain the addresses of .entry label definitions.
 * 
 * @return returns an integer value. If the file is successfully created and written to, it returns `RC_OK`.
 */
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

/**
 * The function `createExtFile` iterates through symbols table, writes external symbols and their usage
 * to a file.
 * 
 * @param filename represents the name of the file to be created. This file will contain the addresses of .extern label calls.
 * 
 * @return returns an integer value. If the file is successfully created and written to, it will return `RC_OK`.
 */
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

/**
 * The function `backendPass` creates output files for object code, entry points, and external
 * references based on the input filename.
 * 
 * @param filename the name of a file(s).
 * 
 * @return returns an integer value. If there is an error during the backend pass process (creating OB, ENT, EXT files), it will return `RC_E_BACKEND_FAILED`. Otherwise,
 * it will return `RC_OK`.
 */
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

    return RC_OK; /*backend successful*/
}