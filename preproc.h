#include <stdio.h>
#include "globals.h"


typedef struct Macro {
    int line_count;
    char macro_name [MAX_TOKEN_LENGTH];
    char * macro_contents;
}Macro;



char * extractMacroName(char* line);

static int getMaxMacroLines(FILE * asFile);

static Macro * initializeMacro (int max_lines);

int scanMacros (FILE * asFile, Macro * macro_table);

Macro * searchMacroName (Macro * macro_table, char * name);

void preasm (char * asFileName);