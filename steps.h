#include "lib.h"

typedef char * t_commandLine[MAX_COMMAND_SIZE];

/* Front-end pass, parsing*/
int parseAssembley (FILE *amFile, AST ** code_ast, AST ** data_ast);

/* middle step */
int middlePass(AST * code_ast, AST * data_ast);

/* backend step, final */
int backendPass(char * filename);
