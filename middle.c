#include "lib.h"
#include "globals.h"
#include "front.h"
#include "datastruct.h"
#include "errors.h"


static int symAllocPTR(Symbols *sym){
    int i = 0;
    sym = (Symbols *)calloc(sizeof(Symbols), 1);
    return RC_OK;
}

int midPassing(AST *ast){
    AST *curr = ast;
    AST *last_node = curr;
    Symbols *symbols;

    int IC = START_ADDRESS;
    int DC = 0;
    int i = 0;
    int line_objects_num = 0;
    char *str;

    while (last_node){ /*first pass*/
        last_node = last_node->next;
    }

    while (curr){
        if (curr->label_occurrence){
            symbols->IC = IC;
            symbols->SymContext = OCCURRENCE;
            symbols->label = my_strdup(curr->label_occurrence, -1);
            IC++;
        }
        else if (curr->cmd_type == DIRECTIVE){
            symbols->IC = IC;
            if (curr->command.directive.type == STRING){
                symbols->SymContext = STRING;
                symbols->sym_data_options.string = curr;
            }
            else if (curr->command.directive.type == DATA){
                symbols->SymContext = DATA;
                symbols->sym_data_options.data = curr;
            }
            else if (curr->command.directive.type == ENTRY){
                symbols->SymContext = ENTRY;
                symbols->label = my_strdup(curr->command.directive.directive_options.label, -1);
            }
            else if (curr->command.directive.type == EXTERN){
                symbols->SymContext = EXTERN;
                symbols->label = my_strdup(curr->command.directive.directive_options.label, -1);
            }
        }
    }
    
    
    while (curr){ /*second pass*/
        switch (curr->cmd_type){
            case INSTRUCTION:
                /* code */
                break;
            case DIRECTIVE:
                break;
            case DEFINE:
                break;
            
            default:
                break;
        }
    }
    
    return RC_OK;
}