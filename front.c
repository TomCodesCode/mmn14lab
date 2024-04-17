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
    int len = strlen(src) + 1;
    if (delta > 0) len = delta;
    char *dst = (char *)malloc(len);
    if (dst != NULL) {
        memcpy(dst, src, len);
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

    if (*line == ';')
    // Check if it's comment line
        return 1;

    while (line[i] != '\0') {
    // Check if character is not printable (including space)
        if (!isprint(line[i++])) {
            return 0; // Found a non-printable character
        }
    }
  return 1; // No non-printable characters found
}

static int islabel (char *str) {
    int length = strlen(str);
    if (str[length - 1] == ":") return 1;
    return 0;
}

static int isRegister (char *str) {
    if ((!strcmp(str, "r1")) || (!strcmp(str, "r2")) || (!strcmp(str, "r3")) || (!strcmp(str, "r4")) || (!strcmp(str, "r5")) ||
        (!strcmp(str, "r6")) || (!strcmp(str, "r7"))) return 1;
    return 0;
}

static int isImmedNumber (char *str) {
    if (*str == '#') {
        if (atoi(str + 1)) return 1;
        else if (*(str + 1) == '0') return 1;
    }
    return 0;
}

static int isDirect(char *str) {
    
}

static int getOpType (char *str) {
    if (isImmedNumber(str)) return IMMEDIATE;
    if (isRegister(str)) return REGISTER;
}

static char** parseLine (char *line) {                 /*get a line, dismantle it into its properties*/
    char *token;
    char *line_cpy [MAX_LABEL_LENGTH];
    char *str [MAX_LABEL_LENGTH];
    char *ptr;
    int i = 0;
    strcpy(line_cpy, line);
    token = strtok(line_cpy, " ,\t[]");
    while (token != NULL) {
        ptr = token;
        token = strtok(NULL, " ,\t[]");
        if (token == NULL) {
            str[i] = my_strdup(ptr, -1);
            break;
        }
        str[i] = my_strdup(ptr, token - ptr);
        str[i][token - ptr] = '\0'; /*Add NULL terminator*/
        i++;
    }
    return str;
}

static int determineType (char *line, AST* currentNode) {
    char **command_line;
    int i = 0;

    while (isspace(*line)) line++;
    command_line = parseLine(line);
    if (!strcmp(command_line[0], ".define")) return DEFINE;

    if (strlen(command_line[0]) < 1) return EMPTY;

    if (!strcmp(command_line[1], ".string")) return DIRECTIVE;
    if (!strcmp(command_line[0], ".entry")) return DIRECTIVE;
    if (!strcmp(command_line[0], ".extern")) return DIRECTIVE;
    if (!strcmp(command_line[1], ".data")) return DIRECTIVE;

    for (i = 0; i < 16; i++){
        if (!strcmp(command_line[0], inst_prop[i].inst)) return INSTRUCTION;
    }
    return EMPTY;
}

AST *createNode(char *line, Labels *labels) {
    char **command_line;
    int type_enum;
    int op_type;
    int i = 0, j = 0;
    AST *ast = (AST *)malloc(sizeof(AST));
    if (ast == NULL) {
        perror("Memory allocation failed (front->createNode)");
        exit(EXIT_FAILURE);
    }
    
    type_enum = determineType(line, ast);
    command_line = parseLine(line);
    
    if (islabel(command_line[0])) { /*saving labels intp Labels struct and deleting from the current str array*/
        strcpy(labels[j++].label_name, command_line[0]); ///number of labels FIXXXX
        free(command_line[0]);
        while (command_line[i] != NULL) {
            if (command_line[i + 1] == NULL) {
                command_line [i] = NULL;
                break;
            }
            command_line[i] = command_line[i + 1];
            i++;
        }
    }
    i= 0;
    switch (type_enum) {
        case INSTRUCTION: {
            ast->line_type = INSTRUCTION;
            while (strcmp(command_line[0], inst_prop[i++].inst)){}
            ast->commands.instruction.inst_type = --i;

            ast->commands.instruction.operands[0].type = 3;
            
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