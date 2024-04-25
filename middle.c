#include "lib.h"

#define ARE_A 0 /*as defined in the booklet*/
#define ARE_R 2
#define ARE_E 1


/** NOTE: ARE code A is 0, so nothing will be done. ARE code E is for externs only- will be assigned in middlePassStep directly.
*This function'll determine if this is an ARE R
 * @param symbol a pointer to a string that represents a symbol.
 * 
 * @return returns an integer (enum) value, which is the result of calling `getSymbolTypeForARE(symbol)`.
 */
int getARE(char * symbol){ 
    int rc = getSymbolTypeForARE(symbol);
    if (rc != RC_OK)
        *symbol = 0;

    return rc;
}

/**
 * The function `middlePassStep` processes an Abstract Syntax Tree (AST) representing code or data,
 * generating opcode lines based on the instructions and directives found in the AST.
 * The function will update the symbol uses for the ouput files, check for errors, and send nodes for opcode calculation.
 * 
 * @param ast a pointer to an AST that represents the parsed code or data.
 * 
 * @return returns `RC_OK` unless there are any errors, then it will return the corresponding error code.
 */
static int middlePassStep(AST *ast) {
    int rc;
    int operand_val_num;
    char *operand_val_str;
    int operand_index;
    int symbol_index;
    int data_index;
    int num_of_operands;
    AST * cur_ast = ast; /*either code or data*/
    int mid_rc = RC_OK;
    
    if (!ast){
        PRINT_ERROR_MSG(RC_E_NO_AST_TABLE);
        return RC_E_NO_AST_TABLE;
    }

    while (cur_ast)
    {
        switch (cur_ast->cmd_type) {
            case INSTRUCTION:
                rc = addOpcode(INSTTYPE, cur_ast->command.instruction.inst_type, TRUE); /*add an opcode line*/
                if (rc != RC_OK) {
                    if (mid_rc == RC_OK) mid_rc = rc;
                    break;
                }
                cur_ast->dc++;

                num_of_operands = numValidInstOperands(cur_ast->command.instruction.inst_type); /*get number of valid operands*/

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
                rc = addOpcode(ARE, ARE_A, FALSE); /*ARE balue for instruction lines is always 00 (A). FALSE- no new opcode is added*/
                if (rc != RC_OK) {
                    if (mid_rc == RC_OK) mid_rc = rc;
                    break;
                }
                
                for (operand_index = 0; operand_index < num_of_operands; operand_index++){ /*for each operand in range for current instruction*/
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
                            cur_ast->dc++; /*update the DC value within the AST*/
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

                            rc = RC_NOT_FOUND;
                            for (i = 0; i < sizeof(sym_type)/sizeof(int); i++) { /*search for the symbol being used as an operand in the table*/
                                rc = getSymbolVal(operand_val_str, sym_type[i], cur_ast->ic + 1, &operand_val_num);
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
                            rc = getARE(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1); /*get ARE value*/
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
                    if (*operand_val_str != '"') { /*check if the string is declared with valid quotations*/
                        PRINT_ERROR_MSG(RC_E_NO_QUOTATION_IN_STR);
                        break;
                    }
                    if (operand_val_str[strlen(operand_val_str)-1] != '"') { /*end quotations*/
                        PRINT_ERROR_MSG(RC_E_NO_QUOTATION_IN_STR);
                        break;
                    }

                    cur_ast->command.directive.directive_options.string.string++;
                    operand_val_str = cur_ast->command.directive.directive_options.string.string;
                    operand_val_str[strlen(operand_val_str)-1] = 0; /*last character in the string is being NULL terminated*/

                    while (*operand_val_str) {
                        addOpcode(VALUE_STR_DATA, *(operand_val_str++), TRUE);
                        cur_ast->dc++;
                    }
                    addOpcode(VALUE_STR_DATA, *operand_val_str, TRUE);
                    cur_ast->dc++;
                }

                else if (cur_ast->command.directive.type == DATA) {
                    data_index = 0; /*index to parse all data objects*/
                    while (data_index != cur_ast->command.directive.data_objects){
                        if (cur_ast->command.directive.directive_options.data[data_index].type == LABEL_DATA){ /*if it's a label, deploy .define*/
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


/**
 * The function `middlePass` processes both Abstract Syntax Trees and prints error messages if any
 * errors occur during the processing.
 * 
 * @param code_ast the AST corresponding to the code (intruction) lines that need to be processed during
 * the middle pass phase.
 * @param data_ast the AST corresponding to the data (data + string) lines that need to be processed during
 * the middle pass phase.
 * 
 * @return `RC_OK` if both `middlePassStep(code_ast)` and `middlePassStep(data_ast)` return successfully without any errors.
 * If either of the `middlePassStep` functions returns an error code, then `RC_E_MIDDLE_PASS_FAILED` is returned.
 */
int middlePass(AST * code_ast, AST *data_ast) {
    int rc1, rc2;

    rc1 = middlePassStep(code_ast);
    PRINT_ERROR_MSG(rc1);
    rc2 = middlePassStep(data_ast);
    PRINT_ERROR_MSG(rc2);
    if (rc1 || rc2)
        return RC_E_MIDDLE_PASS_FAILED;

    return RC_OK;
}
        