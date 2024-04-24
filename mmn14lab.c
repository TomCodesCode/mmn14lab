#include "lib.h"

int IC_START = START_ADDRESS;
int DC_START = 0;
int PC = START_ADDRESS;
int DC = 0;

char *getcwd(char *buf, size_t size);

int main(int argc, char *argv[]){ 
    char cwd[128];
    FILE *amFile = NULL;
    char * filename = "test.asm";
    AST * code_ast;
    AST * data_ast;
    int rc;

    if (argc > 1)
        filename = argv[1];

    amFile = fopen(filename, "r");

    getcwd(cwd, sizeof(cwd));
    
    if (!amFile)
    {
        printf("File not found: No '%s' in [%s]\n", filename, cwd);
        return -1;
    }
    
    rc = parseAssembley(amFile, &code_ast, &data_ast);
    if (rc) exit (rc);

    rc = middlePass(code_ast, data_ast);
    if (rc) exit (rc);

    rc = backendPass(filename);
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
