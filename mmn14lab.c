#include "lib.h"


int IC = START_ADDRESS;
int DC = 0;

char *getcwd(char *buf, size_t size);

int main(int argc, char *argv[]){ 
    char cwd[128];
    FILE *amFile = NULL;
    char * filename = "test.asm";
    AST * code_ast;
    AST * data_ast;
    /*Opcodes * code_opcode_tbl;
    Opcodes * data_opcode_tbl;*/
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
    if (rc != RC_OK) 
        exit (rc);

    dumpSymbolTbl();

    midPassing(code_ast);

    dumpSymbolTbl();

    midPassing(data_ast);

    dumpSymbolTbl();

    fclose(amFile);
    return 0;
}
