#include "lib.h"



int midPassing(AST *ast) {

    AST * cur_ast = ast;

    if (!ast)
        return RC_E_NO_AST_TABLE;

    while (cur_ast)
    {
        switch (cur_ast->cmd_type) {
            case INSTRUCTION:
            case DIRECTIVE:
            case DEFINE:
            default:
                break;
        }
    }
    
    return RC_OK;
}
