#include "lib.h"
#include "globals.h"
#include "preproc.h"

#define MAX_MACROS_NUMBER 20


int * curr_read_position = 0;

/*
typedef struct Macro {
    int line_count;
    char macro_name [MAX_LABEL_LENGTH];
    char * macro_contents;
}Macro;*/

char * extractMacroName(char* line) {

    int contents_length;
    char* contents;
    char* space_position = strchr(line, ' ');
    if (space_position == NULL) {
        return "";
    }

    contents_length = strlen(space_position + 1);
    contents = malloc(contents_length + 1);
    strcpy(contents, space_position + 1);

    return contents;
}


static int getMaxMacroLines(FILE * asFile){

    int max = 0;
    int current_macro_lines;
    char line [MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, asFile) != NULL)
    {
        current_macro_lines = 0;
        
        if(strstr(line, "mcr ") != NULL) {
            while (1) {
                current_macro_lines++;
                if (strstr(fgets(line, MAX_LINE_LENGTH, asFile), "endmcr") != NULL) break;
            }
        }

        if(current_macro_lines > max) max = current_macro_lines;
    }

    fseek(asFile, 0, SEEK_SET);
    
    return max;
}

static Macro * initializeMacro (int max_lines) {
    
    int i;
    Macro * macro_table;

    macro_table = malloc(MAX_MACROS_NUMBER * sizeof (Macro));
    for (i = 0; i < MAX_MACROS_NUMBER; i++) {
        macro_table[i].macro_contents = malloc(MAX_LINE_LENGTH * max_lines * sizeof (char));
    }
    
    return macro_table;
}


int scanMacros (FILE * asFile, Macro * macro_table) {
    
    char line [MAX_LINE_LENGTH];
    int i = 0;
    int count;
    
    while (fgets(line, MAX_LINE_LENGTH, asFile))
    {
        count = 0;

        if (strstr(line, "mcr ") != NULL) {
        
            macro_table[i].macro_contents = "";
            strcpy(macro_table[i].macro_name, extractMacroName(line)); /* get the name of the current macro*/
        
            while (1) {
                if (fgets(line, MAX_LINE_LENGTH, asFile) != NULL) {
                    if (strstr(line, "endmcr") != NULL) break;
                    strcatMalloc(macro_table[i].macro_contents, line);
                }
                continue;
            }

            macro_table[i++].line_count = count;

            /**curr_read_position = ftell(asFile);*/
        }
        
    }

    fseek(asFile, 0, SEEK_SET);

    return 0;
}

Macro * searchMacroName (Macro * macro_table, char * name) {
    int i;
    char * ptr;
    while (isspace(*name)) {
            name++;
            }
    for (i = 0; i < MAX_MACROS_NUMBER; i++) {
        ptr = macro_table[i].macro_name;
        while (isspace(*ptr)) {
            ptr++;
        }
        while (!isspace(*ptr) && !isspace(*name))
        {
            if (*ptr != *name) break;
            ptr++;
            name++;
            if ((isspace(*ptr) && !isspace(*name)) || (!isspace(*ptr) && isspace(*name))) break;
            if (isspace(*ptr) && isspace(*name)) return &macro_table[i];
        }
    }

    return NULL;
}

void preasm (char * asFileName) {
    
    char * amFileName = "";
    FILE * asFile;
    FILE * amFile;
    char line [MAX_LINE_LENGTH];
    Macro * macro_table;
    Macro * macro_ptr;
    int max_lines;
    int rc;

    strcatMalloc(amFileName, asFileName);
    strcatMalloc(asFileName, ".as");
    strcatMalloc(amFileName, ".am");
    asFile = fopen(asFileName, "r");
    amFile = fopen(amFileName, "w");

    max_lines = getMaxMacroLines(asFile);

    macro_table = initializeMacro(max_lines);
    
    rc = scanMacros(asFile, macro_table);

    if (rc != 0) printf ("FAILED!");
    

    while (fgets(line, MAX_LINE_LENGTH, asFile)) {
        if (strstr(line, "mcr ") != NULL) {
            while (fgets(line, MAX_LINE_LENGTH, asFile) && strstr(line, "endmcr") == NULL) {}
            continue;            
        }

        macro_ptr = searchMacroName(macro_table, line);
        
        if (macro_ptr) {
            fputs(macro_ptr->macro_contents , amFile);
            continue;
        }

        fputs(line, amFile);
    }

    fclose(asFile);
    fclose(amFile);
    free(asFileName);
    return;
}
/*
int main() {

    preasm("ps");
    return 0;
}
*/