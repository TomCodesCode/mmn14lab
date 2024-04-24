
enum {
    RC_OK = 0,
    RC_NOT_FOUND,
    RC_I_last,
    RC_E_ALLOC_FAILED,
    RC_E_INVALID_OPERAND,
    RC_E_INVALID_TYPE,
    RC_E_INVALID_CMD,
    RC_E_FAILED_RETRIEVE_OPERANDS,
    RC_E_UNINITIALIZED_SYM_TBL,
    RC_E_UNINITIALIZED_Opcode_TBL,
    RC_E_DUPLICATED_SYMBOL,
    RC_E_NO_AST_TABLE,
    RC_E_LABEL_NOT_DEFINED_BFR_USE,
    RC_E_NO_QUOTATION_IN_STR,
    RC_E_NO_DIRECTIVE_ARGUMENT,
    RC_E_TOO_MANY_ARGUMENTS,
    RC_E_TOO_FEW_ARGUMENTS,
    RC_E_MIDDLE_PASS_FAILED,
    RC_E_last
} RC_CODE;

extern const char *errorMsg[];

#define PRINT_ERROR_MSG(rc) if (rc) printf("ERROR: [%s:%d] %s\n", __FILE__, __LINE__, errorMsg[rc])
#define PRINT_MSG_STR(str)  printf("\t>>> %s\n", str)
#define PRINT_MSG_VAL(val)  printf("\t>>> %d\n", val)
