#include "lib.h"
#include "front.h"
#include "datastruct.h"


typedef struct Instructions {
    char *inst;
    int opcode;
    char *src;
    char *dest;
}Instructions;


Instructions inst_prop[16] = {{"mov", 0, "0123", "123"}, {"cmp", 1, "0123", "0123"}, {"add", 2, "0123", "123"}, {"sub", 3,"0123", "123"},
                            {"not", 4, "-", "123"}, {"clr", 5, "-", "123"}, {"lea", 6, "12", "123"}, {"inc", 7, "-", "123"},
                            {"dec", 8, "-", "123"}, {"jmp", 9, "-", "13"}, {"bne", 10, "-", "13"}, {"red", 11, "-", "123"}, 
                            {"prn", 12, "-", "0123"}, {"jsr", 13, "-", "13"}, {"rts", 14, "-", "-"}, {"hlt", 15, "-", "-"}};


int error_count = 0;

static char *my_strdup(const char *src, int delta) {
    int len = (delta > 0) ? delta : strlen(src) + 1;
    char *dst = (char *)malloc(len);
    if (dst != NULL) {
        memcpy(dst, src, len - 1);
        dst[len - 1] = '\0';
    }
    return dst;
}


static int isEmptyLine(char *line) {
    int i = 0;
    while (isspace(line[i]) && line[i] != '\0') {
        i++;
        if (line[i] == '\0')
            return 1;
    }
    return 0;
}

int isSkipLine(const char *line) {

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

char ** dismantleOperand(char *str) {
    char opParts[2][MAX_LABEL_LENGTH];
    
    char *ptr1 = strchr(str, '[');
    char *ptr2 = strchr(str, ']');
    
    strncpy(opParts[0], str, ptr1 - str - 1);
    opParts[0][ptr1 - str - 1] = '\0';
    strncpy(opParts[1], ptr1 + 1, ptr2 - ptr1 - 1);
    opParts[1][ptr2 - ptr1 - 1] = '\0';

    return opParts;
}

static int islabel (char *str) {
    int length = strlen(str);
    if (str[length - 1] == ":") return 1;
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
    int i = 0;
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

static int getOperandType (char *str) {
    if (isImmediate(str)) return IMMEDIATE;
    if (isDirect(str)) return DIRECT;
    if (isIndexNum(str)) return INDEX_NUM;
    if (isIndexLabel(str)) return INDEX_LABEL;
    if (isRegister(str)) return REGISTER;
    return -1;
}

static char** parseLine (char *line) {                 /*get a line, dismantle it into its properties*/
    char *token;
    char *line_cpy [MAX_LABEL_LENGTH];
    char *str [MAX_LABEL_LENGTH];
    char *ptr;
    int i = 0;
    strcpy(line_cpy, line);
    token = strtok(line_cpy, " ,\t");
    while (token != NULL) {
        ptr = token;
        token = strtok(NULL, " ,\t");
        if (token == NULL) {
            str[i] = my_strdup(ptr, -1);
            break;
        }
        str[i] = my_strdup(ptr, token - ptr);
        i++;
    }
    return str;
}

static int determineType (char *line, char **command_line, AST *curr) {
    int i = 0;
    char *ptr;

    while (isspace(*line)) line++;
    if (!strcmp(command_line[0], ".define")) return DEFINE;

    if (strlen(command_line[0]) < 1) return EMPTY;

    if (!strcmp(command_line[1], ".string")){
        curr->line_type = DIRECTIVE;
        curr->commands.directive.type = STRING;
        ptr = command_line[2] + 1; /*ignoring the "" in the string delaration*/
        curr->commands.directive.directive_options.string = my_strdup(ptr, strchr(ptr, '"') - ptr);
        return DIRECTIVE;
    }
    if (!strcmp(command_line[0], ".entry")) return DIRECTIVE;
    if (!strcmp(command_line[0], ".extern")) return DIRECTIVE;
    if (!strcmp(command_line[1], ".data")){
        curr->line_type = DIRECTIVE;
        curr->commands.directive.type = DATA;
        return DIRECTIVE;
    }

    for (i = 0; i < 16; i++){
        if (!strcmp(command_line[0], inst_prop[i].inst)) return INSTRUCTION;
    }
    return EMPTY;
}

AST *createNode(char *line, Labels *labels) {
    char **command_line;
    char *opPart1;
    int type_enum;
    int op_type;
    int i = 0;
    AST *ast = (AST *)malloc(sizeof(AST));
    if (ast == NULL) {
        perror("Memory allocation failed (front->createNode)");
        exit(EXIT_FAILURE);
    }
    
    command_line = parseLine(line);
    type_enum = determineType(line, command_line, ast);

    if (islabel(command_line[0]) && type_enum != DIRECTIVE) { /*saving label name into the ast and deleting from the current str array*/
        ast->label_occurrence = my_strdup(command_line[0], -1);
        free(command_line[0]);
        while (command_line[i] != NULL) {
            if (command_line[i + 1] == NULL) {
                command_line [i] = NULL;
                break;
            }
            command_line[i] = command_line[i + 1];
            i++;
        }
    }else{ast->label_occurrence = NULL;} /*no label occurrance on this line, NULL the pointer*/

    i= 0;
    switch (type_enum) {
        case INSTRUCTION: {
            ast->line_type = INSTRUCTION;
            while (strcmp(command_line[0], inst_prop[i++].inst) && i < 16);
            ast->commands.instruction.inst_type = --i;
            op_type = getOperandType(command_line[1]);
            ast->commands.instruction.operands[0].type = op_type;
            switch (op_type) {
                case IMMEDIATE: {
                    ast->commands.instruction.operands[0].operand_select.immediate = atoi(command_line[1] + 1);
                    break;
                }
                case DIRECT: {
                    ast->commands.instruction.operands[0].operand_select.label = my_strdup(command_line[1], -1);
                    break;
                }
                case INDEX_NUM: { //better way to approach this??
                    ast->commands.instruction.operands[0].operand_select.index_op.label1 = my_strdup(dismantleOperand(command_line[1])[0], -1);
                    opPart1 = my_strdup(dismantleOperand(command_line[1])[1], -1);
                    ast->commands.instruction.operands[0].operand_select.index_op.index_select.number = atoi(opPart1);
                    free(opPart1);
                    break;
                }
                case INDEX_LABEL: {
                    ast->commands.instruction.operands[0].operand_select.index_op.label1 = my_strdup(dismantleOperand(command_line[1])[0], -1);
                    opPart1 = my_strdup(dismantleOperand(command_line[1])[1], -1);
                    ast->commands.instruction.operands[0].operand_select.index_op.index_select.label2 = my_strdup(opPart1, -1);
                    free(opPart1);
                    break;
                }
                case REGISTER: {
                    ast->commands.instruction.operands[0].operand_select.reg = atoi(command_line[1] + 1);
                    break;
                }
                default:
                    break;
            }
            ast->commands.instruction.operands[0].operand_select.index_op.index_select.number = 1;
            break;
        }
        case DIRECTIVE:
            break;
        case DEFINE:
            break;
        case EMPTY:
            break;    
        default:
            break;
    }
    error_count = 0;
    return ast;
}

static char *checkIfLabel(char *line) {
    int i = 0;
    char *ptr;
    char str [MAX_LINE_LENGTH];
    
    strcpy(str, line);
    ptr = strchr(str, ':');
    if (!ptr) return NULL;
    while (str + i != ptr) {
        if (!isalnum(*(str + i))) return NULL;
        i++;
    }
    strncpy(str, ptr - i, i);
    return str;
}

int getLabelsParse(FILE *amFile, Labels *labels) {
    int i = 0;
    char line [MAX_LINE_LENGTH];
    char *ptr;
    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        ptr = line;
        while (isspace(*ptr)) ptr++;
        if (checkIfLabel(ptr)) {

        }
    }
    fseek(amFile, 0, SEEK_SET);
}

AST *parseAssembley (FILE *amFile) {
    AST *head = NULL;
    AST *current = NULL;
    AST *newnode = NULL;
    Labels *labels = (Labels *)malloc(MAX_LABELS * sizeof(Labels));
    int rc;
    int i = 0;
    int type_enum;
    char line [MAX_LINE_LENGTH];

    //rc = getLabelsParse(amFile, labels);

    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        //type_enum = determineType(line, current);
        if (isEmptyLine(line)) continue;
        newnode = createNode(line, labels);

        if (!newnode)
            return NULL;

        if (!head)
            head = newnode;
        else
            current->next = newnode;

        current = newnode;
    }
    
}