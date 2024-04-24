#include "lib.h"

#define ARE_A 0 /*as defined in the booklet*/
#define ARE_R 2
#define ARE_E 1


/* ARE code A is 0, so nothing will be done. ARE code E is for externs only- will be assigned in midPassing directly.
*This function'll determine if this is an ARE R
*/
int getARE(char * symbol){ 
    int rc;
    
    rc = getSymbolTypeForARE(symbol);
    if (rc != RC_OK)
        PRINT_ERROR_MSG(rc);
        return rc;
    
    return RC_OK;
}

int midPassing(AST *ast) {
    int rc;
    int operand_val_num;
    char *operand_val_str;
    int operand_index;
    int symbol_index;
    int data_index;
    int num_of_operands;
    AST * cur_ast = ast;
    int mid_rc = RC_OK;
    
    if (!ast){
        PRINT_ERROR_MSG(RC_E_NO_AST_TABLE);
        return RC_E_NO_AST_TABLE;
    }

    while (cur_ast)
    {
        switch (cur_ast->cmd_type) {
            case INSTRUCTION:
                rc = addOpcode(INSTTYPE, cur_ast->command.instruction.inst_type, TRUE);
                if (rc != RC_OK) {
                    if (mid_rc == RC_OK) mid_rc = rc;
                    break;
                }
                cur_ast->dc++;

                num_of_operands = numValidInstOperands(cur_ast->command.instruction.inst_type);

                if (num_of_operands == 2) {
                    rc = addOpcode(OPERAND_2_TYPE, getOpcodeTypeByOperand(cur_ast->command.instruction.operands[1].type), FALSE);
                    if (rc != RC_OK) {
                        if (mid_rc == RC_OK) mid_rc = rc;
                        break;
                    }
                }
                
                if (num_of_operands >= 1) {
                    int op_type = OPERAND_1_TYPE;
                    if (num_of_operands == 1) op_type = OPERAND_2_TYPE;
                    rc = addOpcode(op_type, getOpcodeTypeByOperand(cur_ast->command.instruction.operands[0].type), FALSE);
                    if (rc != RC_OK) {
                        if (mid_rc == RC_OK) mid_rc = rc;
                        break;
                    }
                }
                rc = addOpcode(ARE, 00, FALSE);
                if (rc != RC_OK) {
                    if (mid_rc == RC_OK) mid_rc = rc;
                    break;
                }
                
                for (operand_index = 0; operand_index < num_of_operands; operand_index++){
                    switch (cur_ast->command.instruction.operands[operand_index].type){

                        case IMMEDIATE_LABEL:
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.label, DEFINEsym, cur_ast->dc, &operand_val_num);
                            if (rc != RC_OK){
                                PRINT_ERROR_MSG(RC_E_LABEL_NOT_DEFINED_BFR_USE);
                                if (mid_rc == RC_OK) mid_rc = rc;
                                break;
                            }
                            cur_ast->command.instruction.operands[operand_index].operand_select.immediate = operand_val_num;
                        /* fall thru */
                        case IMMEDIATE_VAL:
                            operand_val_num = cur_ast->command.instruction.operands[operand_index].operand_select.immediate;
                            
                            rc = isValidOperand(cur_ast->command.instruction.inst_type, cur_ast->command.instruction.operands[operand_index].type, operand_index);
                            if (rc != RC_OK) { /*check if this is a legal use of an operand within the current inst type*/
                                if (mid_rc == RC_OK) mid_rc = rc;
                                break;
                            }
                            
                            rc = addOpcode(VALUE, operand_val_num, TRUE);
                            if (rc != RC_OK) {
                                if (mid_rc == RC_OK) mid_rc = rc;
                                break;
                            }
                            cur_ast->dc++;
                            symbol_index++;
                            break;

                        case DIRECT:
                        {
                            int sym_type[] = { STRINGsym, DATAsym, EXTERNsym, CODEsym };
                            int i;
                            operand_val_str = cur_ast->command.instruction.operands[operand_index].operand_select.label;
                            
                            rc = isValidOperand(cur_ast->command.instruction.inst_type, cur_ast->command.instruction.operands[operand_index].type, operand_index);
                            if (rc != RC_OK) { /*check if this is a legal use of an operand within the current inst type*/
                                if (mid_rc == RC_OK) mid_rc = rc;
                                break;
                            }

                            rc = RC_NOT_FOUND; /*NO NEED FOR THIS*/
                            for (i = 0; i < sizeof(sym_type)/sizeof(int); i++) {
                                rc = getSymbolVal(operand_val_str, sym_type[i], cur_ast->dc, &operand_val_num);
                                if (rc == RC_OK){
                                    if (sym_type[i] == EXTERNsym) operand_val_num = 0;
                                    rc = addOpcode(VALUE, operand_val_num, TRUE);
                                    cur_ast->dc++;
                                    if (sym_type[i] == EXTERNsym) rc = addOpcode(ARE, ARE_E, FALSE);
                                    break;
                                }
                            }
                            if (rc != RC_OK) {
                                PRINT_ERROR_MSG(RC_NOT_FOUND);
                                printf("symbol '%s' not found\n", operand_val_str);
                                if (mid_rc == RC_OK) mid_rc = RC_NOT_FOUND;
                            }
                            rc = getARE(cur_ast->command.instruction.operands[operand_index].operand_select.label);
                            if (rc == RC_OK) /* checking if the ARE code is R*/
                                addOpcode(ARE, ARE_R, FALSE);
                            break;
                        }
                        case INDEX_NUM:
                            rc = isValidOperand(cur_ast->command.instruction.inst_type, cur_ast->command.instruction.operands[operand_index].type, operand_index);
                            if (rc != RC_OK) { /*check if this is a legal use of an operand within the current inst type*/
                                if (mid_rc == RC_OK) mid_rc = rc;
                                break;
                            }

                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, DATAsym, cur_ast->dc, &operand_val_num);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, TRUE);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, STRINGsym, cur_ast->dc, &operand_val_num);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, TRUE);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, EXTERNsym, cur_ast->dc, &operand_val_num);
                            if (rc == RC_OK) {
                                rc = addOpcode(VALUE, 0, TRUE);
                                rc = addOpcode(ARE, ARE_E, FALSE);
                            }
                            cur_ast->dc++;
                            rc = getARE(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1);
                            if (rc == RC_OK) /* checking if the ARE code is R*/
                                addOpcode(ARE, ARE_R, FALSE);
                            operand_val_num = cur_ast->command.instruction.operands[operand_index].operand_select.index_op.index_select.number;
                            rc = addOpcode(VALUE, operand_val_num, TRUE); /*second part of operand (index)*/
                            cur_ast->dc++;
                            if (rc != RC_OK) mid_rc = RC_E_ALLOC_FAILED;
                            break;

                        case INDEX_LABEL:
                            rc = isValidOperand(cur_ast->command.instruction.inst_type, cur_ast->command.instruction.operands[operand_index].type, operand_index);
                            if (rc != RC_OK) { /*check if this is a legal use of an operand within the current inst type*/
                                if (mid_rc == RC_OK) mid_rc = rc;
                                break;
                            }

                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, DATAsym, cur_ast->dc, &operand_val_num);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, TRUE);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, STRINGsym, cur_ast->dc, &operand_val_num);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, TRUE);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, EXTERNsym, cur_ast->dc, &operand_val_num);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, TRUE);
                            cur_ast->dc++;
                            rc = getARE(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1);
                            if (rc == RC_OK) /* checking if the ARE code is R*/
                                addOpcode(ARE, ARE_R, FALSE);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.index_select.label2, DEFINEsym, cur_ast->dc, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, TRUE); /*second part of operand (index)*/
                            cur_ast->dc++;
                            break;

                        case REGISTER:
                            rc = isValidOperand(cur_ast->command.instruction.inst_type, cur_ast->command.instruction.operands[operand_index].type, operand_index);
                            if (rc != RC_OK) { /*check if this is a legal use of an operand within the current inst type*/
                                if (mid_rc == RC_OK) mid_rc = rc;
                                break;
                            }

                            if (operand_index == 1 && cur_ast->command.instruction.operands[operand_index].type == REGISTER){
                                rc = addOpcode(REGISTER_2, cur_ast->command.instruction.operands[operand_index].operand_select.reg, FALSE);
                                break;
                            }
                            rc = addOpcode(REGISTER_1, cur_ast->command.instruction.operands[operand_index].operand_select.reg, TRUE);
                            cur_ast->dc++;
                            break;
                        default:
                            break;
                    }
                }
                break;

            case DIRECTIVE:
                if (cur_ast->command.directive.type == STRING){
                    operand_val_str = cur_ast->command.directive.directive_options.string.string;
                    if (!operand_val_str) {
                        PRINT_ERROR_MSG(RC_E_INVALID_OPERAND);
                        break;
                    }
                    if (*operand_val_str != '"') {
                        PRINT_ERROR_MSG(RC_E_NO_QUOTATION_IN_STR);
                        break;
                    }
                    if (operand_val_str[strlen(operand_val_str)-1] != '"') {
                        PRINT_ERROR_MSG(RC_E_NO_QUOTATION_IN_STR);
                        break;
                    }

                    cur_ast->command.directive.directive_options.string.string++;
                    operand_val_str = cur_ast->command.directive.directive_options.string.string;
                    operand_val_str[strlen(operand_val_str)-1] = 0;

                    while (*operand_val_str) {
                        addOpcode(VALUE_STR_DATA, *(operand_val_str++), TRUE);
                        cur_ast->dc++;
                    }
                    addOpcode(VALUE_STR_DATA, *operand_val_str, TRUE);
                    cur_ast->dc++;
                }

                else if (cur_ast->command.directive.type == DATA) {
                    data_index = 0;
                    while (data_index != cur_ast->command.directive.data_objects){
                        if (cur_ast->command.directive.directive_options.data[data_index].type == LABEL_DATA){
                            rc = getSymbolVal(cur_ast->command.directive.directive_options.data[data_index].data_options.label, DEFINEsym, cur_ast->dc, &operand_val_num);
                            if (rc == RC_OK)
                                cur_ast->command.directive.directive_options.data[data_index].data_options.number = operand_val_num;
                            else{
                                printf ("Label used in data: %s -> ", cur_ast->command.directive.directive_options.data[data_index].data_options.label);
                                PRINT_ERROR_MSG(RC_NOT_FOUND);
                                mid_rc = RC_NOT_FOUND;
                            }
                        }
                        addOpcode(VALUE_STR_DATA, cur_ast->command.directive.directive_options.data[data_index].data_options.number, TRUE);
                        cur_ast->dc++;
                        data_index++;
                    }
                }
            default:
                break;
        }
        cur_ast = cur_ast->next;
    }
    
    if (mid_rc != RC_OK){
        printf ("\nNo output files. Exiting. Reason: -> ");
        PRINT_ERROR_MSG(mid_rc);
        return mid_rc;
    }

    return RC_OK;
}
