#include "lib.h"

typedef char * t_commandLine[MAX_COMMAND_SIZE];

int parseAssembley (FILE *amFile, AST ** code_ast, AST ** data_ast);
