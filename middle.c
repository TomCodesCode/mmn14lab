#include "lib.h"

static int mid_rc = RC_OK;
static Opcodes * opcodes_arr = NULL;
#define OPCODE_TABLE_BULK_SIZE 20
static int num_of_opcodes = 0;

static int initOpcodesTbl(void) {
    Opcodes * opcodes_arr_tmp = NULL;
    if (!(num_of_opcodes % OPCODE_TABLE_BULK_SIZE)) {
        if (!opcodes_arr)
            opcodes_arr_tmp = (Opcodes *)calloc(sizeof(Opcodes), OPCODE_TABLE_BULK_SIZE);
        else
            opcodes_arr_tmp = (Opcodes *)realloc(opcodes_arr, (num_of_opcodes + OPCODE_TABLE_BULK_SIZE)*sizeof(Opcodes));

        opcodes_arr = opcodes_arr_tmp;
    }
    if (!opcodes_arr) {
        PRINT_ERROR_MSG(RC_E_UNINITIALIZED_SYM_TBL);
        return RC_E_UNINITIALIZED_SYM_TBL;
    }
    return RC_OK;
}

int addOpcode(int wordtype, int num, int opcode_index){
    int rc;
    int neg_bool = FALSE;
    Opcodes opcode_cpy;
    rc = initOpcodesTbl();
    
    opcode_cpy = opcodes_arr[opcode_index];

    switch (wordtype){
        case ARE:
            opcodes_arr[opcode_index].opcode |= (num & 0x3);
            break;
        
        case OPERAND_2_TYPE:
            opcodes_arr[opcode_index].opcode |= ((num & 0x3) << 2);
            break;
        
        case OPERAND_1_TYPE:
            opcodes_arr[opcode_index].opcode |= ((num & 0x3) << 4);
            break;
        
        case INSTTYPE:
            opcodes_arr[opcode_index].opcode |= ((num & 0xf) << 6);
            break;
        
        case VALUE:
            if (num < 0){
                neg_bool = TRUE;
                num = (-1) * num;
            }
            opcodes_arr[opcode_index].opcode |= (num & 0xf) << 2;
            
            if (neg_bool){
                num = ~num;
                num++;
            }
            break;

        case REGISTER_1:
            opcodes_arr[opcode_index].opcode |= (num & 0x7) << 2;

        case REGISTER_2:
            opcodes_arr[opcode_index].opcode |= (num & 0x7) << 4;

        default:
            break;
    }
    opcodes_arr[opcode_index].opcode |= (opcode_cpy.opcode & 0xa);
    num_of_opcodes++;

    return rc;
}


Opcodes * midPassing(AST *ast) {
    int rc;
    int operand_val_num;
    char *operand_val_str;
    /*int operand_str_len;*/
    int operand_index;
    int symbol_index;
    int num_of_operands;
    int opcode_index = 0;
    /*int data_index = 0;*/
    AST * cur_ast = ast;

    if (!ast){
        PRINT_ERROR_MSG(RC_E_NO_AST_TABLE);
        return NULL;
    }

    while (cur_ast)
    {
        switch (cur_ast->cmd_type) {
            case INSTRUCTION:
                rc = addOpcode(INSTTYPE, cur_ast->command.instruction.inst_type, opcode_index);
                rc = addOpcode(OPERAND_2_TYPE, cur_ast->command.instruction.operands[1].type, opcode_index);
                rc = addOpcode(OPERAND_1_TYPE, cur_ast->command.instruction.operands[0].type, opcode_index);
                rc = addOpcode(ARE, 00, opcode_index);
                num_of_operands = numValidInstOperands(cur_ast->command.instruction.inst_type);
                opcode_index++;
                
                for (operand_index = 0; operand_index < num_of_operands; operand_index++){
                    switch (cur_ast->command.instruction.operands[operand_index].type){

                        case IMMEDIATE_LABEL:
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.label, DEFINEsym, cur_ast->ic, &operand_val_num);
                            if (rc != RC_OK){
                                PRINT_ERROR_MSG(RC_E_LABEL_NOT_DEFINED_BFR_USE);
                                break;
                            }
                            cur_ast->command.instruction.operands[operand_index].operand_select.immediate = operand_val_num;
                        /* fall thru */
                        case IMMEDIATE_VAL:
                            operand_val_num = cur_ast->command.instruction.operands[operand_index].operand_select.immediate;
                            rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            symbol_index++;
                            break;

                        case DIRECT:
                        {
                            int sym_type[] = { STRINGsym, DATAsym, EXTERNsym, CODEsym };
                            int i;
                            operand_val_str = cur_ast->command.instruction.operands[operand_index].operand_select.label;
                            rc = RC_NOT_FOUND;
                            for (i = 0; i < sizeof(sym_type)/sizeof(int); i++) {
                                rc = getSymbolVal(operand_val_str, sym_type[i], cur_ast->ic, &operand_val_num);
                                if (rc == RC_OK){
                                    rc = addOpcode(VALUE, operand_val_num, opcode_index);
                                    break;
                                }
                            }
                            if (rc != RC_OK) {
                                PRINT_ERROR_MSG(RC_NOT_FOUND);
                                printf("symbol '%s' not found\n", operand_val_str);
                                mid_rc = RC_NOT_FOUND;
                            }
                            break;
                        }
                        case INDEX_NUM:
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, DATAsym, cur_ast->ic, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, STRINGsym, cur_ast->ic, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, EXTERNsym, cur_ast->ic, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, opcode_index); /*first part of operand (label)*/
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            opcode_index++;
                            operand_val_num = cur_ast->command.instruction.operands[operand_index].operand_select.index_op.index_select.number;
                            rc += addOpcode(VALUE, operand_val_num, opcode_index); /*second part of operand (index)*/
                            if (rc != RC_OK) mid_rc = RC_E_ALLOC_FAILED;
                            break;

                        case INDEX_LABEL:
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, DATAsym, cur_ast->ic, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, STRINGsym, cur_ast->ic, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.label1, EXTERNsym, cur_ast->ic, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, opcode_index); /*first part of operand (label)*/
                            if (rc == RC_OK)
                                rc = addOpcode(VALUE, operand_val_num, opcode_index);
                            opcode_index++;
                            rc = getSymbolVal(cur_ast->command.instruction.operands[operand_index].operand_select.index_op.index_select.label2, DEFINEsym, cur_ast->ic, &operand_val_num);
                            rc = addOpcode(VALUE, operand_val_num, opcode_index); /*second part of operand (index)*/
                            break;

                        case REGISTER:
                            if (operand_index == 1 && cur_ast->command.instruction.operands[operand_index].type == REGISTER){
                                rc = addOpcode(REGISTER_2, cur_ast->command.instruction.operands[operand_index].operand_select.reg, opcode_index);
                                break;
                            }
                            rc = addOpcode(REGISTER_1, cur_ast->command.instruction.operands[operand_index].operand_select.reg, opcode_index);
                            break;
                        default:
                            break;
                    }
                    opcode_index++;
                }
                break;

            case DIRECTIVE:
                if (cur_ast->command.directive.type == STRING){
                    while (operand_val_str){
                        rc = addOpcode(VALUE, *operand_val_str++, opcode_index++);
                    }
                }
                else if (cur_ast->command.directive.type == DATA){
                    /* code */
                }
            default:
                break;
        }
        cur_ast = cur_ast->next;
    }
    
    return opcodes_arr;
}
