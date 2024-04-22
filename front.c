#include "lib.h"


char *my_strdup(const char *src, int delta) {
    int len = (delta > 0) ? (delta + 1) : strlen(src) + 1;
    char *dst = (char *)malloc(len);

    if (dst == NULL) {
        PRINT_ERROR_MSG(RC_E_ALLOC_FAILED);
        exit(RC_E_ALLOC_FAILED);
    }

    memcpy(dst, src, len - 1);
    dst[len - 1] = '\0';

    return dst;
}

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

static int isImmediate (char *str) {
    int i = 0;

    if (*str != '#' || *str == 0)
        return FALSE;

    i++;

    if (str[i] == '+' || str[i] == '-' || isdigit(str[i])) {
        i++;
        for (; str[i]; i++) {
            if (!isdigit(str[i]))
                return FALSE;
        }
    }
    else if (isalpha(str[i++])) {
        for (; str[i]; i++) {
            if (!isalnum(str[i]))
                return FALSE;
        }
    }
    else {
        return FALSE;
    }

    return TRUE;
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
    if (isImmediate(str))
        *operand_type = IMMEDIATE;
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

static int parseLine (char *line, t_commandLine arr) {                 /*get a line, dismantle it into its properties*/
    char *token;
    char *line_cpy;
    char *token_delim = " =,\t\n\r";
    int i = 0;

    for (i = 0; i < MAX_COMMAND_SIZE; i++) {
        arr[i] = NULL;
    }

    line_cpy = my_strdup(line, -1);
    strcpy(line_cpy, line);

    i =  0;

    token = strtok(line_cpy, token_delim);
    while (token != NULL) {
        if (strlen(token) > MAX_TOKEN_LENGTH) {
            /* token is too long */
            return -1;
        }
        arr[i] = (char*)malloc(strlen(token)+1);
        strcpy(arr[i], token);

        token = strtok(NULL, token_delim);
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
        case IMMEDIATE: {
            if (atoi(command_line[1 + operand_idx] + 1) || (*(command_line[1 + operand_idx] + 1) == '0')) { /*check if the immediate is a number or label type*/
                ast->command.instruction.operands[operand_idx].operand_select.immediate = atoi(command_line[1 + operand_idx] + 1);
            }else{
                ast->command.instruction.operands[operand_idx].operand_select.label = my_strdup(command_line[1 + operand_idx], -1);
            }
            IC++;
            break;
        }
        case DIRECT: {
            ast->command.instruction.operands[operand_idx].operand_select.label = my_strdup(command_line[1 + operand_idx], -1);
            IC++;
            break;
        }
        case INDEX_NUM: {
            rc = dismantleOperand(command_line[1 + operand_idx], str); /*separate the operand and the index*/
            if (rc != RC_OK) break;
            ast->command.instruction.operands[operand_idx].operand_select.index_op.label1 = my_strdup(str[0], -1);
            ast->command.instruction.operands[operand_idx].operand_select.index_op.index_select.number = atoi(str[1]);
            IC+=2;
            free(str[0]);
            free(str[1]);
            break;
        }
        case INDEX_LABEL: {
            rc = dismantleOperand(command_line[1 + operand_idx], str); /*separate the operand and the index*/
            if (rc != RC_OK) break;
            ast->command.instruction.operands[operand_idx].operand_select.index_op.label1 = my_strdup(str[0], -1);
            ast->command.instruction.operands[operand_idx].operand_select.index_op.index_select.label2 = my_strdup(str[1], -1);
            IC+=2;
            free(str[0]);
            free(str[1]);
            break;
        }
        case REGISTER: {
            ast->command.instruction.operands[operand_idx].operand_select.reg = atoi(command_line[1 + operand_idx] + 1);
            if (operand_idx == 0)
                IC++;
            else if (operand_idx == 1) {
                if (ast->command.instruction.operands[0].type != REGISTER)
                    IC++;
            }
            break;
        }
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
    int num_of_operands = 0;
    int rc;
    AST *ast;
    
    ast = (AST *)calloc(sizeof(AST), 1);
    if (ast == NULL) {
        perror("Memory allocation failed (front->createNode)");
        exit(EXIT_FAILURE);
    }

    rc = parseLine(line, command_line); /*check for error code*/
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
        ast->label_occurrence = my_strdup(command_line[0], -1);
        rc = deleteFirstString(command_line); /*check for error code*/
    }

    ast->cmd_type = type_enum;

    switch (type_enum) {
        case INSTRUCTION: {
            num_of_operands = numValidInstOperands(ast->command.instruction.inst_type);

            for (i = 0; i < num_of_operands; i++) {
                rc = instOperatorPush(ast, command_line, i, type_enum);
                if (rc != RC_OK)
                   PRINT_ERROR_MSG(RC_E_FAILED_RETRIEVE_OPERANDS);
            }
            IC++;

            break;
        }
        case DIRECTIVE: {
            switch (ast->command.directive.type)
            {
            case STRING:
                ptr = command_line[1] + 1;
                ast->command.directive.directive_options.string.string = my_strdup(ptr, strchr(ptr, '"') - ptr); /*ignoring the "" in the string delaration*/
                break;
            
            case ENTRY:
            case EXTERN:
                ast->command.directive.directive_options.label = my_strdup(command_line[1], -1);
                break;
            
            case DATA:
                i = 0;
                ptr = command_line[1];
                while (ptr){
                    if (isprint(*ptr)){
                        if (strtol(ptr, &endptr, 10) || *ptr == '0'){
                            ast->command.directive.directive_options.data[i].data_options.number = (int)strtol(ptr, &endptr, 10);
                        }else{
                            ast->command.directive.directive_options.data[i].data_options.label = my_strdup(ptr, -1);
                        }
                        i++;
                        ptr = command_line[1 + i];
                    }
                    else printf ("Error: Not an alpha-numeric data type.");
                }

                break;
            
            default:
                break;
            }

            break;
        }
        case DEFINE:{
            ast->command.define.label = my_strdup(command_line[1], -1);
            ast->command.define.number = atoi(command_line[2]);
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

    *code_ast = NULL;
    *data_ast = NULL;

    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        int cmd_ic = IC;
        newnode = createNode(line);

        if (!newnode)
        {
            PRINT_ERROR_MSG(RC_E_ALLOC_FAILED);
            return RC_E_ALLOC_FAILED;
        }

        if (newnode->cmd_type == EMPTY) continue;

        switch (newnode->cmd_type){
            case INSTRUCTION:
                if (newnode->label_occurrence)
                    addSymbolVal(newnode->label_occurrence, CODEsym, cmd_ic);
                
                if (!code_head)
                    code_head = newnode;
                else
                    code_current->next = newnode;
                
                code_current = newnode;
                break;
            
            case DIRECTIVE:
                cmd_type = newnode->command.directive.type;
                if (cmd_type == STRING)
                    addSymbolVal(newnode->command.directive.directive_options.label, STRINGsym, 00000000000);
                else if (cmd_type == ENTRY || cmd_type == EXTERN){
                    if (cmd_type == ENTRY)
                        addSymbolVal(newnode->command.directive.directive_options.label, ENTRYsym, 00000000000);
                    else
                        addSymbolVal(newnode->command.directive.directive_options.label, EXTERNsym, 00000000000);
                }
                else if (cmd_type == DATA){
                    addSymbolVal(newnode->command.directive.directive_options.label, DATAsym, 00000000000);
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

    *code_ast = code_head;
    *data_ast = data_head;

    return RC_OK;
}

