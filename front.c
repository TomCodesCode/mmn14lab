#include "lib.h"
#include "front.h"


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


AST *createNode(AST *ast, int type_enum) {
    ast = (AST *)malloc(sizeof(AST));
    if (ast == NULL) {
        perror("Memory allocation failed (front->createNode)");
        exit(EXIT_FAILURE);
    }
    switch (type_enum) {
        case INSTRUCTION: {
            /* code */
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
}


static char *my_strdup(const char *src, int delta) {
    int len = strlen(src) + 1;
    if (delta > 0) len = delta;
    char *dst = (char *)malloc(len);
    if (dst != NULL) {
        memcpy(dst, src, len);
    }
    return dst;
}


static int islabel(char *str) {
}


static int isnumber(char *str, int max, int min, int result) {
}


AST get_ast_from_line (char *line) {
    AST ast = {0};
    return ast;
}

static char** parseLine (char *line) {                 /*get a line, dismantle it into its properties*/
    char *token;
    char *str [MAX_LABEL_LENGTH];
    char *ptr;
    int i = 0;
    token = strtok(line, " ,\t[]");
    while (token != NULL) {
        ptr = token;
        token = strtok(NULL, " ,\t[]");
        if (token == NULL) {
            str[i] = my_strdup(ptr, -1);
            strcpy(str[i], ptr);
            break;
        }
        str[i] = my_strdup(ptr, token - ptr);
        str[i][token - ptr] = '\0'; /*Add NULL terminator*/
        i++;
    }

    return str;
}


static int determineType (char *line) {
    char **command_line;
    while (isspace(*line)) line++;
    command_line = parseLine(line);
    return DIRECTIVE;
    return INSTRUCTION;
    return EMPTY;
    return DEFINE;
}

AST *parseAssembley (FILE *amFile) {
    AST *head = NULL;
    AST *current = NULL;
    AST *newnode = NULL;
    int rc;
    int i = 0;
    int type_enum;
    char line [MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        type_enum = determineType(line);
        newnode = createNode(current, type_enum);
        if (!head) {
            head = newnode;
            current = head->next;
        }else{
            current = current->next;
        }
    }
    
}