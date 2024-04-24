#include "lib.h"

static int errors_found = FALSE; /* the program will termionate after front is done if this is TRUE*/

static int isSkipLine(const char *line){
    int i = 0;
    if (!line)
        return TRUE;

    if (*line == ';') /*Check if it's comment line*/
        return TRUE;

    while (line[i] != '\0') { /*Check if character is not printable (including space)*/
        if (isprint(line[i]) && line[i] != ' ' ) { 
            return FALSE;
        }
        i++;
    }
    return TRUE;
}

static int dismantleOperand(char *src, char **str) { /*separate the label and the index*/
    char *ptr1;
    char *ptr2;

    str[0] = (char*)malloc(MAX_TOKEN_LENGTH);
    str[1] = (char*)malloc(MAX_TOKEN_LENGTH);

    if (str[0] == NULL || str[1] == NULL) {
        PRINT_ERROR_MSG(RC_E_ALLOC_FAILED);
        exit(RC_E_ALLOC_FAILED);
    }

    ptr1 = strchr(src, '[');
    ptr2 = strchr(src, ']');

    strncpy(str[0], src, ptr1 - src);
    str[0][ptr1 - src] = '\0';
    strncpy(str[1], ptr1 + 1, ptr2 - ptr1 - 1);
    str[1][ptr2 - ptr1 - 1] = '\0';

    return RC_OK;
}

static int islabel (char *str) {
    int length = strlen(str);
    if (str[length - 1] == ':')
        return TRUE;

    return FALSE;
}


static int isImmediateVal (char *str) {
    int i = 0;

    if (*str != '#') return FALSE;

    i++;
    if (str[i] == '+' || str[i] == '-') i++;

    if (isdigit(str[i++])) {
        for (; str[i]; i++) {
            if (!isdigit(str[i]))
                return FALSE;
        }
        return TRUE;
    }
    
    return FALSE;
}

static int isImmediateLabel (char *str) {
    int i = 0;

    if (*str != '#') return FALSE;

    i++;
    if (isalpha(str[i++])) {
        for (; str[i]; i++) {
            if (!isalnum(str[i]))
                return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}

static int isRegister (char *str) {
    if (str[REG_NAME_LEN])
        return FALSE; /* token is too long to be register */

    if (str[0] == 'r' && str[1] >= '0' && str[1] <= '7')
        return TRUE;

    return FALSE;
}

static int isDirect(char *str) {
    int i = 0;

    if (isRegister(str))
        return FALSE;

    while ((i < strlen(str)) && isalnum(*(str + i))) {
        i++;
    }
    if (i == strlen(str))
        return TRUE;
    return FALSE;
}

static int isIndexNum (char *str) {
    char *ptr1 = strchr(str, '[');
    char *ptr2 = strchr(str, ']');
    if (ptr1 && ptr2 && (ptr1 > str) && (isdigit(*(ptr1 + 1))) && *(ptr2+1) == 0) {
        return TRUE;
    }

    return FALSE;
}

static int isIndexLabel (char *str) {
    char *ptr1 = strchr(str, '[');
    char *ptr2 = strchr(str, ']');
    if (ptr1 && ptr2 && (ptr1 > str) && (isalpha(*(ptr1 + 1))) && *(ptr2+1) == 0) {
        return TRUE;
    }

    return FALSE;
}

static int deleteFirstString(t_commandLine str) {
    int i;

    for (i = 0; i < MAX_COMMAND_SIZE-1; i++) {
        str[i] = str[i+1];
    }
    str[i] = NULL;

    return RC_OK;
}

static int getOperandType (char *str, int * operand_type) {
    if (isImmediateVal(str))
        *operand_type = IMMEDIATE_VAL;
    else if (isImmediateLabel(str))
        *operand_type = IMMEDIATE_LABEL;
    else if (isDirect(str))
        *operand_type = DIRECT;
    else if (isIndexNum(str))
        *operand_type = INDEX_NUM;
    else if (isIndexLabel(str))
        *operand_type = INDEX_LABEL;
    else if (isRegister(str))
        *operand_type = REGISTER;
    else {
        PRINT_ERROR_MSG(RC_E_INVALID_OPERAND);
        return RC_E_INVALID_OPERAND;
    }

    return RC_OK;
}

static char * getStringToken(char * str) {
    char * str_token = NULL;

    while (*str) {
        if (!isspace(*str))
            break;
        str++;
    }
    if (*str != '"')
        return NULL;

    str_token = str++;

    while (*str) {
        if (*str == '"' && isspace(*(str+1))) {
            *(str + 1) = 0;
            return str_token;
        }
        str++;
    }
    
    return NULL;
}

static int parseLine (char *line, t_commandLine arr, int * n_tokens) {                 /*get a line, dismantle it into its properties*/
    char *token;
    char *line_cpy;
    char *token_delim = TOKEN_DELIMITERS;
    int i = 0;
    char * str_token;

    *n_tokens = 0;

    for (i = 0; i < MAX_COMMAND_SIZE; i++) {
        arr[i] = NULL;
    }

    line_cpy = my_strdup(line, -1);
    strcpy(line_cpy, line);

    i =  0;

    token = strtok(line_cpy, token_delim);
    while (token != NULL) {
        (*n_tokens)++;

        if (strlen(token) > MAX_TOKEN_LENGTH) {
            /* token is too long */
            return -1;
        }
        arr[i] = (char*)malloc(strlen(token)+1);
        strcpy(arr[i], token);

        if (!strcmp(token, ".string")) {
            str_token = getStringToken(token + strlen(token) + 1);
            if (str_token == NULL) {
                PRINT_ERROR_MSG(RC_E_NO_DIRECTIVE_ARGUMENT);
                PRINT_MSG_STR(line);
                errors_found = TRUE;
                return RC_E_NO_DIRECTIVE_ARGUMENT;
            }
            (*n_tokens)++;

            arr[++i] = (char*)malloc(strlen(str_token)+1);
            strcpy(arr[i], str_token);

            token = strtok(str_token + strlen(str_token) + 1, token_delim);
        }
        else {
            token = strtok(NULL, token_delim);
        }
        i++;
    }
    free(line_cpy);

    return RC_OK;
}

static int determineType (char *line, t_commandLine command_line, AST *curr) {
    int i = 0;
    int token_idx = 0;

    while (isspace(*line)) line++;

    if (isSkipLine(line) || strlen(command_line[token_idx]) == 0)
        return EMPTY;

    if (!strcmp(command_line[token_idx], ".define"))
        return DEFINE;

    if (islabel(command_line[token_idx])) {
        token_idx++;
    }

    if (!strcmp(command_line[token_idx], ".string")) {
        if (token_idx)
            curr->command.directive.directive_options.string.label = my_strdup(command_line[0], -1);

        curr->command.directive.type = STRING;
        return DIRECTIVE;
    }
    if (!strcmp(command_line[token_idx], ".entry")) {
        if (token_idx) {
            printf("Warning: Label found before an entry declaration. Ignoring label.");
        }
        curr->command.directive.type = ENTRY;
        return DIRECTIVE;
    }
    if (!strcmp(command_line[token_idx], ".extern")) {
        if (token_idx) {
            printf("Warning: Label found before an extern declaration. Ignoring label.");
        }
        curr->command.directive.type = EXTERN;
        return DIRECTIVE;
    }
    if (!strcmp(command_line[token_idx], ".data")) {
        if (token_idx) {
            curr->command.directive.directive_options.data->label = my_strdup(command_line[0], -1);
        }
        curr->command.directive.type = DATA;
        return DIRECTIVE;
    }

    for (i = 0; i < INST_SET_SIZE; i++) {
        if (!strcmp(command_line[token_idx], getInstByIdx(i))) {
            curr->command.instruction.inst_type = i;
            return INSTRUCTION;
        }
    }

    return EMPTY;
}

static int instOperatorPush(AST *ast, char **command_line, int operand_idx, int opTypeEnum) {
    int op_type;
    char *str[NUM_OF_OPERANDS];
    int rc;

    rc = getOperandType(command_line[1 + operand_idx], &op_type);
    if (rc != RC_OK)
        return rc;

    ast->command.instruction.operands[operand_idx].type = op_type;
    switch (op_type) {
        case IMMEDIATE_VAL:
            ast->command.instruction.operands[operand_idx].operand_select.immediate = atoi(command_line[1 + operand_idx] + 1);
            PC++;
            break;
        case IMMEDIATE_LABEL:
            ast->command.instruction.operands[operand_idx].operand_select.label = my_strdup(command_line[1 + operand_idx] + 1, -1);
            PC++;
            break;
        case DIRECT:
            ast->command.instruction.operands[operand_idx].operand_select.label = my_strdup(command_line[1 + operand_idx], -1);
            PC++;
            break;
        case INDEX_NUM:
            rc = dismantleOperand(command_line[1 + operand_idx], str); /*separate the operand and the index*/
            if (rc != RC_OK) break;
            ast->command.instruction.operands[operand_idx].operand_select.index_op.label1 = my_strdup(str[0], -1);
            ast->command.instruction.operands[operand_idx].operand_select.index_op.index_select.number = atoi(str[1]);
            PC+=2;
            free(str[0]);
            free(str[1]);
            break;
        case INDEX_LABEL:
            rc = dismantleOperand(command_line[1 + operand_idx], str); /*separate the operand and the index*/
            if (rc != RC_OK) break;
            ast->command.instruction.operands[operand_idx].operand_select.index_op.label1 = my_strdup(str[0], -1);
            ast->command.instruction.operands[operand_idx].operand_select.index_op.index_select.label2 = my_strdup(str[1], -1);
            PC+=2;
            free(str[0]);
            free(str[1]);
            break;
        case REGISTER:
            ast->command.instruction.operands[operand_idx].operand_select.reg = atoi(command_line[1 + operand_idx] + 1);
            if (operand_idx == 0)
                PC++;
            else if (operand_idx == 1) {
                if (ast->command.instruction.operands[0].type != REGISTER)
                    PC++;
            }
            break;
        default:
            break;
    }
    if (rc != RC_OK){
        PRINT_ERROR_MSG(rc);
        return rc;
    }

    return RC_OK;
}

AST *createNode(char *line) {
    t_commandLine command_line;
    char *ptr;
    char *endptr;
    int type_enum;
    int i = 0;
    int data_objects = 0;
    int num_of_operands = 0;
    int rc;
    int n_tokens = 0;
    AST *ast;
    
    ast = (AST *)calloc(sizeof(AST), 1);
    if (ast == NULL) {
        perror("Memory allocation failed (front->createNode)");
        exit(EXIT_FAILURE);
    }

    rc = parseLine(line, command_line, &n_tokens); /*check for error code*/
    if (rc != RC_OK) {
        free(ast);
        PRINT_ERROR_MSG(RC_E_INVALID_CMD);
        return NULL;
    }

    type_enum = determineType(line, command_line, ast);

    if (type_enum == EMPTY) {
        ast->cmd_type = EMPTY;
        return ast;
    }

    if (islabel(command_line[0])) { /*saving label name into the ast and deleting from the current str array*/
        n_tokens--;
        ast->label_occurrence = my_strdup(command_line[0], -1);
        rc = deleteFirstString(command_line); /*check for error code*/
    }

    ast->cmd_type = type_enum;

    switch (type_enum) {
        case INSTRUCTION:
            num_of_operands = numValidInstOperands(ast->command.instruction.inst_type);

            if (num_of_operands > (n_tokens - 1)) {
                PRINT_ERROR_MSG(RC_E_TOO_FEW_ARGUMENTS);
                PRINT_MSG_STR(line);
                errors_found = TRUE;
                break;
            }

            for (i = 0; i < num_of_operands; i++) {
                rc = instOperatorPush(ast, command_line, i, type_enum);
                if (rc != RC_OK)
                   PRINT_ERROR_MSG(RC_E_FAILED_RETRIEVE_OPERANDS);
            }

            if (n_tokens > (num_of_operands + 1)) {
                PRINT_ERROR_MSG(RC_E_TOO_MANY_ARGUMENTS);
                PRINT_MSG_STR(line);
                errors_found = TRUE;
            }

            PC++;

            break;
        
        case DIRECTIVE: {
            switch (ast->command.directive.type)
            {
                case STRING:
                    ptr = command_line[1];
                    endptr = ptr;
                    while (endptr && isprint(*endptr)) endptr++;
                    ast->command.directive.directive_options.string.string = my_strdup(ptr, endptr - ptr); /*allocate and cpy string to AST*/

                    DC += strlen(ast->command.directive.directive_options.string.string) - 2;  /* not counting quotations */
                    DC++; /* for null terminator */

                    if (n_tokens > 2) {
                        PRINT_ERROR_MSG(RC_E_TOO_MANY_ARGUMENTS);
                        PRINT_MSG_STR(line);
                        errors_found = TRUE;
                    }

                    break;
                
                case ENTRY:
                case EXTERN:
                    ast->command.directive.directive_options.label = my_strdup(command_line[1], -1);
                    if (n_tokens > 2) {
                        PRINT_ERROR_MSG(RC_E_TOO_MANY_ARGUMENTS);
                        PRINT_MSG_STR(line);
                        errors_found = TRUE;
                    }
                    break;
                
                case DATA:
                    i = 0;
                    ptr = command_line[1];
                    while (ptr){
                        if (isprint(*ptr)){
                            data_objects++;
                            if (strtol(ptr, &endptr, 10) || *ptr == '0'){
                                ast->command.directive.directive_options.data[i].data_options.number = (int)strtol(ptr, &endptr, 10);
                                ast->command.directive.directive_options.data[i].type = NUMBER_DATA;
                            }else{
                                ast->command.directive.directive_options.data[i].data_options.label = my_strdup(ptr, -1);
                                ast->command.directive.directive_options.data[i].type = LABEL_DATA;
                            }
                            i++;
                            ptr = command_line[1 + i];
                        }else{
                            printf ("Error: Not an alpha-numeric data type.");
                            PRINT_MSG_STR(line);
                            errors_found = TRUE;
                        }
                    }
                    ast->command.directive.data_objects = data_objects;
                    DC += i;

                    break;
                
                default:
                    break;
            }

            break;
        }
            case DEFINE:{
                ptr = command_line[2];
                endptr = NULL;
                if ((strtol(ptr, &endptr, 10) || *ptr == '0') || *endptr == 0){
                    ast->command.define.label = my_strdup(command_line[1], -1);
                    ast->command.define.number = atoi(command_line[2]);
                }else{
                    printf ("Error: Not a valid '.define' value. Lable: %s\n", command_line[1]);
                    PRINT_MSG_STR(line);
                    ast->cmd_type = EMPTY;
                    errors_found = TRUE;
                }
                if (n_tokens > 3) {
                    PRINT_ERROR_MSG(RC_E_TOO_MANY_ARGUMENTS);
                    PRINT_MSG_STR(line);
                    errors_found = TRUE;
                }
                break;
            }
            default:
                break;
    }

    return ast;
}

int parseAssembley (FILE *amFile, AST ** code_ast, AST ** data_ast) {
    AST *data_head = NULL;
    AST *data_current = NULL;
    AST *code_head = NULL;
    AST *code_current = NULL;
    AST *newnode = NULL;
    char line [MAX_LINE_LENGTH];
    int cmd_type = 0;
    AST *ast_tmp;

    *code_ast = NULL;
    *data_ast = NULL;
    
    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        int cmd_pc = PC;
        int cmd_dc = DC;

        newnode = createNode(line);

        if (!newnode)
        {
            PRINT_ERROR_MSG(RC_E_ALLOC_FAILED);
            errors_found = TRUE;
            continue;
            /*return RC_E_ALLOC_FAILED;*/
        }

        if (newnode->cmd_type == EMPTY) continue;

        newnode->ic = cmd_pc;
        newnode->dc = cmd_dc;
        switch (newnode->cmd_type){
            case INSTRUCTION:
                if (newnode->label_occurrence)
                    addSymbolVal(newnode->label_occurrence, CODEsym, cmd_pc);
                
                if (!code_head)
                    code_head = newnode;
                else
                    code_current->next = newnode;
                
                code_current = newnode;
                break;
            
            case DIRECTIVE:
                cmd_type = newnode->command.directive.type;
                if (cmd_type == STRING) {
                    addSymbolVal(newnode->command.directive.directive_options.label, STRINGsym, cmd_dc);
                }
                else if (cmd_type == ENTRY || cmd_type == EXTERN) {
                    if (cmd_type == ENTRY)
                        addSymbolVal(newnode->command.directive.directive_options.label, ENTRYsym, cmd_dc);
                    else
                        addSymbolVal(newnode->command.directive.directive_options.label, EXTERNsym, cmd_dc);
                }
                else if (cmd_type == DATA) {
                    addSymbolVal(newnode->command.directive.directive_options.label, DATAsym, cmd_dc);
                }

                if (!data_head)
                    data_head = newnode;
                else
                    data_current->next = newnode;
                
                data_current = newnode;
                break;
            
            case DEFINE:
                addSymbolVal(newnode->command.define.label, DEFINEsym, newnode->command.define.number);
                
                if (!data_head)
                    data_head = newnode;
                else
                    data_current->next = newnode;
                
                data_current = newnode;
                break;

            default:
                break;
        }
    }

    ast_tmp = data_head;
    DC_START = PC;
    for (ast_tmp = data_head; ast_tmp; ast_tmp = ast_tmp->next) {
        int symbol_type;
        ast_tmp->dc += DC_START;
        cmd_type = ast_tmp->command.directive.type;
        if (cmd_type == STRING)
            symbol_type = STRINGsym;
        else if (cmd_type == ENTRY || cmd_type == EXTERN) {
            if (cmd_type == ENTRY)
                symbol_type = ENTRYsym;
            else
                symbol_type = EXTERNsym;
        }
        else if (cmd_type == DATA)
            symbol_type = DATAsym;
        else
            continue;
        
        updateSymbolVal(ast_tmp->command.directive.directive_options.label, symbol_type, ast_tmp->dc);
    }

    PC += DC;

    *code_ast = code_head;
    *data_ast = data_head;

    if (errors_found == TRUE){
        return RC_E_INVALID_CMD;
    }

    return RC_OK;
}

