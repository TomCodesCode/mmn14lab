#include <string.h>
#include "lib.h"
#include "front.h"
#include "datastruct.h"
#include "errors.h"

char *getcwd(char *buf, size_t size);

int main(int argc, char *argv[]){ 
    char cwd[128];
    FILE *amFile = NULL;
    char * filename = "test.asm";

    if (argc > 1)
        filename = argv[1];

    amFile = fopen(filename, "r");

    getcwd(cwd, sizeof(cwd));
    
    if (!amFile)
    {
        printf("File not found: No '%s' in [%s]\n", filename, cwd);
        return -1;
    }
    parseAssembley(amFile);

    dumpSymbolTbl();

    fclose(amFile);
    return 0;
}
