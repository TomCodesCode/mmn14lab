#include "lib.h"

int IC_START = START_ADDRESS;
int DC_START = 0;
int PC = START_ADDRESS;
int DC = 0;


int main(int argc, char *argv[]) { 
    char am_filename[MAX_FILENAME_LEN];
    char base_filename[MAX_FILENAME_LEN];
    char * asm_filename = NULL;
    FILE * amFile = NULL;
    AST * code_ast;
    AST * data_ast;
    char * pchar;
    int rc;

    if (argc < 2) {
        PRINT_ERROR_MSG(RC_E_NO_INPUT_FILE);
        exit (RC_E_NO_INPUT_FILE);
    }

    asm_filename = argv[1];

    strcpy(base_filename, asm_filename);
    
    pchar = strrchr(base_filename, '.');
    if (pchar)
        *pchar = 0;

    strcpy(am_filename, base_filename);
    rc = preasm(asm_filename, am_filename);
    if (rc) exit (rc);

    amFile = fopen(am_filename, "r");

    rc = parseAssembley(amFile, &code_ast, &data_ast);
    if (rc) exit (rc);

    rc = middlePass(code_ast, data_ast);
    if (rc) exit (rc);

    rc = backendPass(base_filename);
    if (rc) exit (rc);

#if 1 /*DEBUG*/
    dumpSymbolTbl();
    dumpOpcodesTbl();
#endif

    printf("IC: %d DC: %d PC %d #instr:%d #data:%d \n",
        IC_START, DC_START, PC, 
        DC_START?(DC_START-IC_START):(PC-IC_START), 
        DC_START?(PC-DC_START):0);

    fclose(amFile);

    return 0;
}
