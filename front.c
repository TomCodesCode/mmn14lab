#include "lib.h"
#include "front.h"
#include "datastruct.h"
#include <string.h>


int error_count = 0;

static char *my_strdup(const char *src, int delta) {
    int len = (delta > 0) ? delta + 1 : strlen(src) + 1;
    char *dst = (char *)malloc(len);
    if (dst != NULL) {
        memcpy(dst, src, len - 1);
        dst[len - 1] = '\0';
    }
    return dst;
}

static int isSkipLine(const char *line) {

    int i = 0;
    if (!line)
        return 0;

    if (*line == ';') /*Check if it's comment line*/
        return 0;

    while (line[i] != '\0') { /*Check if character is not printable (including space)*/
        if (!isprint(line[i++])) { /*Found a non-printable character*/
            return 0;
        }
    }
    return 1; /*No non-printable characters found*/
}

static char ** dismantleOperand(char *src, char **str) { /*separate the label and the index*/
    char *ptr1;
    char *ptr2;
    
    str[0] = (char*)malloc(MAX_LABEL_LENGTH);
    str[1] = (char*)malloc(MAX_LABEL_LENGTH);

    ptr1 = strchr(src, '[');
    ptr2 = strchr(src, ']');
    
    strncpy(str[0], src, ptr1 - src - 1);
    str[0][ptr1 - src - 1] = '\0';
    strncpy(str[1], ptr1 + 1, ptr2 - ptr1 - 1);
    str[1][ptr2 - ptr1 - 1] = '\0';

    return str;
}

static int islabel (char *str) {
    int length = strlen(str);
    if (str[length - 1] == ':') return 1;
    return 0;
}

static int isImmediate (char *str) {
    if (*str == '#') {
        if (isalnum(*(str + 1))) return 1;
        else if (*(str + 1) == '0') return 1;
    }
    return 0;
}

static int isDirect(char *str) {
    int i = 0;
    while ((i < strlen(str)) && isalnum(*(str + i))) {
        i++;
    }
    if (i == strlen(str)) return 1;
    return 0;
}

static int isIndexNum (char *str) {
    char *ptr1 = strchr(str, '[');
    char *ptr2 = strchr(str, ']');
    if ((ptr1 > str) && ptr1 && ptr2) {
        ptr1++;
        if (atoi(ptr1)) return 1;
    }
    return 0;
}

static int isIndexLabel (char *str) {
    int i = 0;
    char *ptr1 = strchr(str, '[');
    char *ptr2 = strchr(str, ']');
    if (ptr1 && ptr2 && (ptr1 > str) && (isalpha(*(ptr1 + 1)))) {
        while (isalnum(*(ptr1 + i))){
            i++;
            if (ptr1 + i == ptr2) return 1;
        }
    }
    return 0;
}

static int isRegister (char *str) {
    if ((0 > strcmp(str, "r1")) || (0 > strcmp(str, "r2")) || (0 > strcmp(str, "r3")) || (0 > strcmp(str, "r4")) || (0 > strcmp(str, "r5")) ||
        (0 > strcmp(str, "r6")) || (0 > strcmp(str, "r7"))) return 1;
    return 0;
}

static int deleteFirstString(char **str) {
    int i = 0;
    free(str[0]);
    while (str[i] != NULL) {
        if (str[i + 1] == NULL) {
            str [i] = NULL;
            break;
        }
        str[i] = str[i + 1];
        i++;
    }
    return 0;
}

static int getOperandType (char *str) {
    if (isImmediate(str)) return IMMEDIATE;
    if (isDirect(str)) return DIRECT;
    if (isIndexNum(str)) return INDEX_NUM;
    if (isIndexLabel(str)) return INDEX_LABEL;
    if (isRegister(str)) return REGISTER;
    return -1;
}

static int parseLine (char *line, char **arr) {                 /*get a line, dismantle it into its properties*/
    char *token;
    /*char line_cpy [MAX_LINE_LENGTH];*/
    char *line_cpy;
    char *ptr;
    char *endptr;
    int i = 0;
    line_cpy = my_strdup(line, -1);
    strcpy(line_cpy, line);
    token = strtok(line_cpy, " =,\t\n\r");
    while (token != NULL) {
        ptr = token;
        token = strtok(NULL, " =,\t\n\r");
        endptr = ptr + 1;
        while(*endptr == ' ' || *endptr == '=' || *endptr == ',' || *endptr == '\t') endptr--;
        if (token == NULL) {
            arr[i] = my_strdup(ptr, -1);
            break;
        }
        arr[i] = my_strdup(ptr, endptr - ptr);
        i++;
    }
    free(line_cpy);
    return 0;
}

static int determineType (char *line, char **command_line, AST *curr) {
    int i = 0;
    int rc = 0;

    while (isspace(*line)) line++;
    
    if (!isSkipLine(line) || strlen(command_line[0]) < 1) return EMPTY;

    if (!strcmp(command_line[0], ".define")) return DEFINE;

    if (islabel(command_line[0])) {
        if (!strcmp(command_line[1], ".string")) {
            curr->commands.directive.directive_options.string.label = my_strdup(command_line[0], -1);
            rc = deleteFirstString(command_line);
            curr->commands.directive.type = STRING;
            return DIRECTIVE;
        }
        if (!strcmp(command_line[1], ".entry")) {
            rc = deleteFirstString(command_line);
            printf("Warning: Label found before an entry declaration. Ignoring label.");
            curr->commands.directive.type = ENTRY;
            return DIRECTIVE;
        }
        if (!strcmp(command_line[1], ".extern")) {
            rc = deleteFirstString(command_line);
            printf("Warning: Label found before an extern declaration. Ignoring label.");
            curr->commands.directive.type = EXTERN;
            return DIRECTIVE;
        }
        if (!strcmp(command_line[1], ".data")) {
            curr->commands.directive.directive_options.data->label = my_strdup(command_line[0], -1);
            rc = deleteFirstString(command_line);
            curr->commands.directive.type = DATA;
            return DIRECTIVE;
        }
    }
    for (i = 0; i < 16; i++){
        if (!strcmp(command_line[0], inst_prop[i].inst)) {
            curr->commands.instruction.inst_type = i;
            return INSTRUCTION;
        }
    }
    if (rc != 0) return -1;
    return EMPTY;
}

static int instOperatorPush(AST *ast, char **command_line, int i, int opTypeEnum, int op_type) {
    char **str = NULL;
    op_type = getOperandType(command_line[1 + i]);
    ast->commands.instruction.operands[0].type = op_type;
    switch (op_type) {
        case IMMEDIATE: {
            ast->commands.instruction.operands[i].operand_select.immediate = atoi(command_line[1 + i] + 1);
            break;
        }
        case DIRECT: {
            ast->commands.instruction.operands[i].operand_select.label = my_strdup(command_line[1 + i], -1);
            break;
        }
        case INDEX_NUM: {
            str = dismantleOperand(command_line[1 + i], str); /*separate the operand and the index*/
            ast->commands.instruction.operands[i].operand_select.index_op.label1 = my_strdup(str[0], -1);
            ast->commands.instruction.operands[i].operand_select.index_op.index_select.number = atoi(str[1]);
            free(str);
            break;
        }
        case INDEX_LABEL: {
            str = dismantleOperand(command_line[1 + i], str); /*separate the operand and the index*/
            ast->commands.instruction.operands[i].operand_select.index_op.label1 = my_strdup(str[0], -1);
            ast->commands.instruction.operands[i].operand_select.index_op.index_select.label2 = my_strdup(str[1], -1);
            free(str);
            break;
        }
        case REGISTER: {
            ast->commands.instruction.operands[i].operand_select.reg = atoi(command_line[1] + 1);
            break;
        }
        default:
            break;
    }
    
    return 0;
}

AST *createNode(char *line) {
    char command_line[MAX_COMMAND_SIZE][MAX_LABEL_LENGTH];
    char *ptr;
    char *endptr;
    int type_enum;
    int op_type = 0;
    int i = 0;
    int rc;
    AST *ast = (AST *)malloc(sizeof(AST));
    if (ast == NULL) {
        perror("Memory allocation failed (front->createNode)");
        exit(EXIT_FAILURE);
    }
    
    rc = parseLine(line, command_line); /*check for error code*/
    type_enum = determineType(line, command_line, ast);

    if (type_enum == EMPTY) ast->line_type = EMPTY;

    if (islabel(command_line[0]) && type_enum != DIRECTIVE) { /*saving label name into the ast and deleting from the current str array*/
        ast->label_occurrence = my_strdup(command_line[0], -1);
        rc = deleteFirstString(command_line); /*check for error code*/
    }else{ast->label_occurrence = NULL;} /*no label occurrance on this line, NULL the pointer*/

    switch (type_enum) {
        case INSTRUCTION: {
            ast->line_type = INSTRUCTION;
            for (i = 0; i < 2; i++){
                rc = instOperatorPush(ast, command_line, i, type_enum, op_type);
                if (rc != 0) printf("Error getting operators.");
            }
            
            break;
        }
        case DIRECTIVE: {
            ast->line_type = DIRECTIVE;
            switch (ast->commands.directive.type)
            {
            case STRING:{
                ptr = command_line[2] + 1;
                ast->commands.directive.directive_options.string.string = my_strdup(ptr, strchr(ptr, '"') - ptr - 1); /*ignoring the "" in the string delaration*/
                break;
            }
            case ENTRY || EXTERN:{
                ast->commands.directive.directive_options.label = my_strdup(command_line[1], -1);
                break;
            }
            case DATA:{
                i = 0;
                ptr = command_line[1];
                while (ptr){
                    if (isprint(*ptr)){
                        if (!strtol(ptr, &endptr, 10)){
                            ast->commands.directive.directive_options.data[i].data_options.number = (int)strtol(ptr,&endptr,10);
                        }else{
                            ast->commands.directive.directive_options.data[i].data_options.label = my_strdup(ptr, -1);
                        }
                        i++;
                        ptr = command_line[1 + i];
                    }
                    else printf ("Error: Not an alpha-numeric data type.");
                }
                
                break;
            }
            default:
                break;
            }

            break;
        }
        case DEFINE:{
            ast->line_type = DEFINE;
            ast->commands.define.label = my_strdup(command_line[1], -1);
            ast->commands.define.number = atoi(command_line[2]);
        }  
        default:
            break;
    }
    
    error_count = 0;
    return ast;
}

AST *parseAssembley (FILE *amFile) {
    AST *head = NULL;
    AST *current = NULL;
    AST *newnode = NULL;
    char line [MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        newnode = createNode(line);

        if (!newnode)
            return NULL;
        
        if (newnode->line_type == EMPTY) continue;

        if (!head)
            head = newnode;
        else
            current->next = newnode;

        current = newnode;
    }

    return head;
}

int main(){
    FILE *amFile = fopen("ps.am", "r");
    parseAssembley(amFile);
    fclose(amFile);
    return 0;
}