
enum {
    RC_OK = 0,
    RC_E_ALLOC_FAILED,
    RC_E_INVALID_OPERAND,
    RC_E_INVALID_TYPE,
    RC_E_INVALID_CMD,
    RC_E_FAILED_RETRIEVE_OPERANDS,
    RC_last
} RC_CODE;

enum {
    FALSE = 0,
    TRUE
} BOOL;

extern const char *errorMsg[];

#define PRINT_ERROR_MSG(rc)  printf("ERROR: [%s:%d] %s\n", __FILE__, __LINE__, errorMsg[rc])
