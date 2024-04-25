#include "lib.h"

static int errors_found = FALSE; /* the program will termionate after front is done if this is TRUE*/

/**
 * The function isSkipLine checks if a given line is a comment line or contains only non-printable
 * characters.
 * 
 * @param line the current line in the file.
 * 
 * @return returns TRUE if the input line is a comment line, otherwise, it returns FALSE.
 */
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

/**
 * The function `dismantleOperand` separates a label and an index from a given source string.
 * 
 * @param src represents the input string containing the label and index enclosed in square brackets.
 * @param str a pointer to a pointer to astring. This function separates a string into two parts - a label and an
 * index - based on the presence of square brackets '[' and ']'.
 * 
 * @return returns `RC_OK` when completed.
 */
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

/**
 * Checks if a given string ends with a colon (meaning a label declaration) and returns TRUE if it does.
 * 
 * @param str a string, which represents the supposed label.
 * 
 * @return returns TRUE if the input string is a label, and FALSE otherwise.
 */
static int islabel (char *str) {
    int length = strlen(str);
    if (str[length - 1] == ':')
        return TRUE;

    return FALSE;
}

/**
 * This function checks if a given operand is an 'immediate val (number)' type.
 * 
 * @param str a string input that represents the operand being checked whether it
 * is a valid 'immediate val' type.
 * 
 * @return returns either TRUE or FALSE based on whether the conditions are met.
 */
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

/**
 * This function checks if a given operand is an 'immediate label' type.
 * 
 * @param str a string input that represents the operand being checked whether it
 * is a valid 'immediate label' type.
 * 
 * @return returns either TRUE or FALSE based on whether the conditions are met.
 */
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

/**
 * This function checks if a given operand is a 'register' type.
 * 
 * @param str a string input that represents the operand being checked whether it
 * is a valid 'register' type.
 * 
 * @return returns either TRUE or FALSE based on whether the conditions are met.
 */
static int isRegister (char *str) {
    if (str[REG_NAME_LEN])
        return FALSE; /* token is too long to be register */

    if (str[0] == 'r' && str[1] >= '0' && str[1] <= '7')
        return TRUE;

    return FALSE;
}

/**
 * This function checks if a given operand is a 'direct' type.
 * 
 * @param str a string input that represents the operand being checked whether it
 * is a valid 'direct' type.
 * 
 * @return returns either TRUE or FALSE based on whether the conditions are met.
 */
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

/**
 * This function checks if a given operand is an 'index number' type.
 * 
 * @param str a string input that represents the operand being checked whether it
 * contains a valid index number in the format "[index]".
 * 
 * @return returns either TRUE or FALSE based on whether the conditions are met.
 */
static int isIndexNum (char *str) {
    char *ptr1 = strchr(str, '[');
    char *ptr2 = strchr(str, ']');
    if (ptr1 && ptr2 && (ptr1 > str) && (isdigit(*(ptr1 + 1))) && *(ptr2+1) == 0) {
        return TRUE;
    }

    return FALSE;
}

/**
 * This function checks if a given operand is an 'index label' type.
 * 
 * @param str a string input that represents the operand being checked whether it
 * contains a valid index label in the format "[index]".
 * 
 * @return returns either TRUE or FALSE based on whether the conditions are met.
 */
static int isIndexLabel (char *str) {
    char *ptr1 = strchr(str, '[');
    char *ptr2 = strchr(str, ']');
    if (ptr1 && ptr2 && (ptr1 > str) && (isalpha(*(ptr1 + 1))) && *(ptr2+1) == 0) {
        return TRUE;
    }

    return FALSE;
}

/**
 * This function deletes the first string in a command line array.
 * 
 * @param str an string array representing a command line. The function deletes
 * the first string in the command line by shifting all strings to the left.
 * 
 * @return returns the status code `RC_OK`.
 */
static int deleteFirstString(t_commandLine str) {
    int i;

    for (i = 0; i < MAX_COMMAND_SIZE-1; i++) {
        str[i] = str[i+1];
    }
    str[i] = NULL;

    return RC_OK;
}

/**
 * The function `getOperandType` determines the type of operand based on the input string.
 * 
 * @param str a pointer to a string that represents an operand.
 * @param operand_typea pointer to an integer where the function will store the type of operand.
 * 
 * @return returns an integer value. If the function executes
 * successfully, it will return the value `RC_OK`. If there is an invalid operand detected, it will return
 * `RC_E_INVALID_OPERAND`.
 */
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

/**
 * The function `getStringToken` extracts a string token enclosed in double quotes from a given input
 * string.
 * 
 * @param str The function takes a string `str` as input and extracts a token enclosed
 * in double quotes from it. The function skips any leading whitespace characters before the opening
 * double quote and returns the token enclosed in double quotes. If the token is successfully
 * extracted, it returns a pointer to the beginning of it.
 * 
 * @return returns a pointer to the beginning of a string token if it
 * finds a valid string. If no valid string token is found, it returns NULL.
 */
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

/**
 * The function `parseLine` takes a line of text, breaks it down into tokens based on delimiters, and
 * handles special cases for certain tokens like ".string".
 * 
 * @param line parses a line of input into tokens based on a set of delimiters. Also handles special cases that require
 * additional processing, such as the ".string" directive.
 * @param arr an array of `t_commandLine` type, used to store the tokens extracted from the input line. The function dismantles the input
 * line into its properties and stores each token in this array for further processing.
 * @param n_tokens a pointer to an integer that stores the number of tokens found in the input line after it has been parsed.
 * 
 * @return integer value. If the function executes successfully, it will return the value `RC_OK`.
 */
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

    token = strtok(line_cpy, token_delim); /*using strtok to separate to line into tokens*/
    while (token != NULL) {
        (*n_tokens)++;

        if (strlen(token) > MAX_TOKEN_LENGTH) {
            /* token is too long */
            return -1;
        }
        arr[i] = (char*)malloc(strlen(token)+1);
        strcpy(arr[i], token);

        if (!strcmp(token, ".string")) { /*since strings can be multiple words, they will ne treated differently*/
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
            token = strtok(NULL, token_delim); /*move on to next string part*/
        }
        i++;
    }
    free(line_cpy);

    return RC_OK;
}

/**
 * The function `determineType` analyzes a command line to determine its type, such as instruction,
 * directive, or empty.
 * 
 * @param line a pointer to a character array, which represents a line of code.
 * @param command_line an array that contains the line's tokens. Accessed using `token_idx` to determine the
 * type of command.
 * @param curr a pointer to an AST (Abstract Syntax Tree) structure.The function analyzes a line of code and determines its
 * type, then inserts the informartion to curr (current AST node).
 * 
 * @return an integer value that represents the type of the command being processed.
 *  The possible return values and their meanings are as follows:
 * - `EMPTY`: Indicates that the line is empty or should be skipped.
 * - `DEFINE`: Indicates that the command is a define directive.
 * - `DIRECTIVE`: Indicates that the command is a directive (e.g., string, entry, extrern...)
 * - 'INSTRUCTION': Indicates that the command is an instruction (e.g., mov, add...)
 */
static int determineType (char *line, t_commandLine command_line, AST *curr) {
    int i = 0;
    int token_idx = 0;

    while (isspace(*line)) line++; /*ignore the whitespace at the beginning*/

    if (isSkipLine(line) || strlen(command_line[token_idx]) == 0)
        return EMPTY; /*no need to parse line, ignore continue*/

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

    for (i = 0; i < INST_SET_SIZE; i++) { /*if this is reached check if it's any instruction*/
        if (!strcmp(command_line[token_idx], getInstByCode(i))) {
            curr->command.instruction.inst_type = i;
            return INSTRUCTION;
        }
    }

    errors_found = TRUE;
    PRINT_ERROR_MSG (RC_E_INVALID_CMD);
    PRINT_MSG_STR(line);
    return EMPTY; /*nothing matched, ignore the line and print the error code.*/
}

/**
 * The function `instOperatorPush` processes and stores operands for an instruction in an abstract
 * syntax tree (AST).
 * 
 * @param ast a pointer to an AST node. Used to store information about the operands of an
 * instruction.
 * @param command_line an array of strings representing the command
 * line input. Each element in the array corresponds to a token or a word from the input command line.
 * @param operand_idx the index of the operand being processed within the instruction. Used to determine which operand
 * of the instruction is currently being handled and to update the corresponding fields in the AST accordingly.
 * @param opTypeEnum the type of operand being processed.
 * 
 * @return returns an integer value, which is either `RC_OK` if
 * the function execution was successful, or an error code if there was an issue during execution.
 */
static int instOperatorPush(AST *ast, char **command_line, int operand_idx, int opTypeEnum) {
    int op_type;
    char *str[NUM_OF_OPERANDS];
    int rc;

    rc = getOperandType(command_line[1 + operand_idx], &op_type);
    if (rc != RC_OK)
        return rc;

    ast->command.instruction.operands[operand_idx].type = op_type;
    switch (op_type) { /*based on operands type, populate the AST node (only for instructions)*/
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

/**
 * The function `createNode` parses a line of code, determines its type, and creates an Abstract Syntax
 * Tree (AST) node based on the parsed information.
 * 
 * @param line The function `createNode` takes a string `line` as input and processes it to create an
 * Abstract Syntax Tree (AST) node. The function parses the input line, determines the type of command,
 * and populates the AST node accordingly based on the command type.
 * 
 * @return An AST node is being returned.
 */
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

    rc = parseLine(line, command_line, &n_tokens); /*parse line and check for error code*/
    if (rc != RC_OK) {
        free(ast);
        PRINT_ERROR_MSG(RC_E_INVALID_CMD);
        return NULL;
    }

    type_enum = determineType(line, command_line, ast); /*get the type of current line*/

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

    switch (type_enum) { /*based on the line's type, populate the AST node*/
        case INSTRUCTION:
            num_of_operands = numValidInstOperands(ast->command.instruction.inst_type); /*get the number of valid operands based on instuction*/

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

            PC++; /*inc total address*/

            break;
        
        case DIRECTIVE: {
            switch (ast->command.directive.type)
            {
                case STRING:
                    ptr = command_line[1];
                    endptr = ptr;
                    while (endptr && isprint(*endptr)) endptr++;
                    ast->command.directive.directive_options.string.string = my_strdup(ptr, endptr - ptr); /*allocate + cpy string to AST*/

                    DC += strlen(ast->command.directive.directive_options.string.string) - 2;  /* not counting quotations */
                    DC++; /* for null terminator */

                    if (n_tokens > 2) {
                        PRINT_ERROR_MSG(RC_E_TOO_MANY_ARGUMENTS);
                        PRINT_MSG_STR(line);
                        errors_found = TRUE;
                    }

                    break;
                
                case ENTRY: /*in this case, entry and extern are being treated the same*/
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
                        if (isprint(*ptr)){ /*check if this is a valid (printable) input*/
                            data_objects++;
                            if (strtol(ptr, &endptr, 10) || *ptr == '0'){ /*for 'pure' number inpouts*/
                                ast->command.directive.directive_options.data[i].data_options.number = (int)strtol(ptr, &endptr, 10);
                                ast->command.directive.directive_options.data[i].type = NUMBER_DATA;
                            }else{ /*the input might be a define label. save for middle parse.*/
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
                    ast->command.directive.data_objects = data_objects; /*update the amount of objects saved in the data label*/
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

/**
 * The function `parseAssembley` reads assembly code from a file, creates abstract syntax trees for
 * code and data sections, and updates symbol values accordingly.
 * 
 * @param amFile The file that contains assembly code to be parsed and processed. The function reads lines from this file.
 * @param code_ast a pointer to a pointer to an Abstract Syntax Tree (AST). Used to store the AST representing the code
 * section of the assembly file being parsed. The function populates this AST with nodes representing instructions.
 * @param data_ast a pointer to a pointer to an AST that represents the data section of an assembly file.
 * @return an integer value. If there are no errors found, it returns `RC_OK`. If errors are found, it returns
 * `RC_E_INVALID_CMD`.
 */
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
        }

        if (newnode->cmd_type == EMPTY) continue; /*an empty / comment line is ignored*/

        /*update current node with currently known addresses*/
        newnode->ic = cmd_pc;
        newnode->dc = cmd_dc;
        switch (newnode->cmd_type){
            case INSTRUCTION:
                if (newnode->label_occurrence)
                    addSymbolVal(newnode->label_occurrence, CODEsym, cmd_pc); /*if a label is declared, store it in symbols table*/
                
                /*creates a new code node with the new information extracted into 'newnode'*/
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
                        addSymbolVal(newnode->command.directive.directive_options.label, EXTERNsym, 0); /*externs don't need a value*/
                }
                else if (cmd_type == DATA) {
                    addSymbolVal(newnode->command.directive.directive_options.label, DATAsym, cmd_dc);
                }
                
                /*creates a new data node with the new information extracted into 'newnode'*/
                if (!data_head)
                    data_head = newnode;
                else
                    data_current->next = newnode;
                
                data_current = newnode;
                break;
            
            case DEFINE:
                addSymbolVal(newnode->command.define.label, DEFINEsym, newnode->command.define.number);
                
                /*create a new data node with the information extracted into 'newnode'. defines'll be in dataAST and get parsed in middle*/
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

    /*after both ASTs were created, the now known number of intruction nodes will be added to the dataAST to update it's new addresses*/
    ast_tmp = data_head;
    DC_START = PC;
    for (ast_tmp = data_head; ast_tmp; ast_tmp = ast_tmp->next) {
        int symbol_type;
        int val;
        ast_tmp->dc += DC_START;
        val = ast_tmp->dc;
        cmd_type = ast_tmp->command.directive.type;
        if (cmd_type == STRING)
            symbol_type = STRINGsym;
        else if (cmd_type == ENTRY || cmd_type == EXTERN) {
            if (cmd_type == ENTRY)
                symbol_type = ENTRYsym;
            else {
                symbol_type = EXTERNsym;
                val = 0;
            }
        }
        else if (cmd_type == DATA)
            symbol_type = DATAsym;
        else
            continue;
        
        updateSymbolVal(ast_tmp->command.directive.directive_options.label, symbol_type, val);
    }

    PC += DC; /*update total address (PC)*/

    *code_ast = code_head;
    *data_ast = data_head;

    /*if there were any erors during the front process, an error is returned and main will not continue to 'middle'*/
    if (errors_found == TRUE){
        return RC_E_INVALID_CMD;
    }

    /*everything was successful during the 'front' run, continue.*/
    return RC_OK;
}

