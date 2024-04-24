#include "errors.h"

const char *errorMsg[] = {
    /*RC_OK */                          "",
    /*RC_NOT_FOUND*/                    "item not found",
    /*RC_I_last*/                       "",
    /*RC_ALLOC_FAILED*/                 "memory allocation failed",
    /*RC_INVALID_OPERAND*/              "invalid operand specified",
    /*RC_INVALID_TYPE*/                 "invalid type",
    /*RC_E_INVALID_CMD */               "failed to parse command line",
    /*RC_E_FAILED_RETRIEVE_OPERANDS */  "failed to retrieve operands from command line",
    /*RC_E_UNINITIALIZED_SYM_TBL*/      "uninitialized symbol table",
    /*RC_E_UNINITIALIZED_Opcode_TBL*/   "uninitialized opcodes table",
    /*RC_E_DUPLICATED_SYMBOL*/          "redefinition of symbol or lable",
    /*RC_E_NO_AST_TABLE*/               "no AST provided, or empty",
    /*RC_E_LABEL_NOT_DEFINED_BFR_USE*/  "label was not defined before use",
    /*RC_E_NO_QUOTATION_IN_STR*/        "no quotation mark in string",
    /*RC_E_NO_DIRECTIVE_ARGUMENT*/      "no argument found in directive",
    /*RC_E_TOO_MANY_ARGUMENTS*/         "too many arguments in command",
    /*RC_E_TOO_FEW_ARGUMENTS*/          "too few arguments in command",
    /*RC_E_MIDDLE_PASS_FAILED*/         "middle pass failed",
    /*last*/                            ""
};
