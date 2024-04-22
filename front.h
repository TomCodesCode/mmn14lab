#include "lib.h"

typedef char * t_commandLine[MAX_COMMAND_SIZE];

char *my_strdup(const char *src, int delta);
int parseAssembley (FILE *amFile, AST ** code_ast, AST ** data_ast);
