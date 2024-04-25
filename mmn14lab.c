#include "lib.h"

int IC_START = START_ADDRESS;
int DC_START = 0;
int PC = START_ADDRESS;
int DC = 0;


int main(int argc, char *argv[]) { /*main function*/
    char am_filename[MAX_FILENAME_LEN];
    char base_filename[MAX_FILENAME_LEN];
    char * asm_filename = NULL;
    FILE * amFile = NULL;
    AST * code_ast;
    AST * data_ast;
    char * pchar;
    int rc; /*error codes*/

    if (argc < 2) { /*check for valid amount of input files*/
        PRINT_ERROR_MSG(RC_E_NO_INPUT_FILE);
        exit (RC_E_NO_INPUT_FILE);
    }

    asm_filename = argv[1];

    strcpy(base_filename, asm_filename);
    
    pchar = strrchr(base_filename, '.');
    if (pchar)
        *pchar = 0;

    strcpy(am_filename, base_filename);
    rc = preasm(asm_filename, am_filename); /*pre-assembeler. get and deploy macros, ouput a new source file*/
    if (rc) exit (rc);

    amFile = fopen(am_filename, "r"); /*open new source file (post macros deployment)*/

    rc = parseAssembley(amFile, &code_ast, &data_ast); /*Frontend- first pass. creates AST and symbol table*/
    if (rc) exit (rc);

    rc = middlePass(code_ast, data_ast); /*Middleend- second pass (2 passes- 1 for dataAST and 1 for codeAST). deploy symbols and update thier uses, check for errors, etc.*/
    if (rc) exit (rc);

    rc = backendPass(base_filename); /*Backend- encrypt opcodes and output files if needed*/
    if (rc) exit (rc);

#if 1 /*DEBUG*/
    dumpSymbolTbl();
    dumpOpcodesTbl();
#endif

    printf("IC: %d DC: %d PC %d #instr:%d #data:%d \n",
        IC_START, DC_START, PC, 
        DC_START?(DC_START-IC_START):(PC-IC_START), 
        DC_START?(PC-DC_START):0);

    fclose(amFile); /*close the source file*/

    return 0;
}
