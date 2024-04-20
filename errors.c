#include "errors.h"

const char *errorMsg[] = {
    /*RC_OK */                          "",
    /*RC_ALLOC_FAILED*/                 "memory allocation failed",
    /*RC_INVALID_OPERAND*/              "invalid operand specified",
    /*RC_INVALID_TYPE*/                 "invalid type",
    /*RC_E_INVALID_CMD */               "failed to parse command line",
    /*RC_E_FAILED_RETRIEVE_OPERANDS */  "failed to retrieve operands from command line",
    /*last*/                            ""
};
