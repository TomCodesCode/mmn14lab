#include "lib.h"

int IC_START = START_ADDRESS;
int DC_START = 0;
int PC = START_ADDRESS;
int DC = 0;


int main(int argc, char *argv[]) { 
    char am_filename[MAX_FILENAME_LEN];
    char * asm_filename = "test.as";
    FILE * amFile = NULL;
    AST * code_ast;
    AST * data_ast;
    int rc;

    if (argc > 1)
        asm_filename = argv[1];

    rc = preasm(asm_filename, am_filename);
    if (rc) exit (rc);

    amFile = fopen(am_filename, "r");

    rc = parseAssembley(amFile, &code_ast, &data_ast);
    if (rc) exit (rc);

    rc = middlePass(code_ast, data_ast);
    if (rc) exit (rc);

    rc = backendPass(asm_filename);
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
