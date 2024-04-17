#include "lib.h"
#include "globals.h"
#include "front.h"
#include "datastruct.h"


typedef struct Labels {
    char *labelname;
    int address;
}Labels [MAX_LABELS];

int fun(AST ast){}

static int countLineParts(char* line) {
    int counter = 0;
    char *token;
    token = strtok(line, " ,\t[]#");
    while (token != NULL) {
        token = strtok(NULL, " ,\t[]#");
        if (token == NULL) break;
        counter++;
    }
    return counter;
}

static int getLabels(FILE *amFile) {
    Labels labels;
    int PC = START_ADDRESS;
    int inc;
    char *line [MAX_LINE_LENGTH];
    char *label_end;
    char *label_start;
    char str[MAX_LABEL_LENGTH];
    int length;
    int i = 0;
    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        inc = countLineParts(line);
        if (!(label_end = strchr(line, ":"))) {
             PC += inc;
             continue;
        }
        label_start = line;
        while (isspace(label_start)) label_start++;
        length = label_end - label_start;
        
        strncpy(str, label_start, length);
        labels[i].labelname[length] = "\0";
        //ptr = strtok(line, " :,\t");
    }
    fseek(amFile, 0, SEEK_SET);
    return 1;
}