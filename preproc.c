#include "lib.h"

#define MAX_MACROS_NUMBER 20

int * curr_read_position = 0;

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

char *getcwd(char *buf, size_t size);

static void closeFiles(FILE * as, FILE * am)
{
    if (as) fclose(as);
    if (am) fclose(am);
}

int preasm (char * asFileName, char * amFileName) {
    char cwd[MAX_FILENAME_LEN];
    FILE * asFile = NULL;
    FILE * amFile = NULL;
    char line [MAX_LINE_LENGTH];
    Macro * macro_table;
    Macro * macro_ptr;
    int max_lines;
    int rc;

    getcwd(cwd, sizeof(cwd));

    if (strlen(asFileName) > MAX_FILENAME_LCHK) {   
        PRINT_ERROR_MSG(RC_E_FILENAME_TOO_LONG);
        PRINT_MSG_STR(asFileName);
        return RC_E_FILENAME_TOO_LONG;
    }

    asFile = fopen(asFileName, "r");
    if (!asFile) {
        PRINT_ERROR_MSG(RC_E_FAILED_TO_OPEN_FILE);
        PRINT_MSG_STR(cwd);
        PRINT_MSG_STR(asFileName);
        return RC_E_FAILED_TO_OPEN_FILE;
    }

    strcpy(amFileName, asFileName);
    strcat(amFileName, FN_AM_EXT);

    amFile = fopen(amFileName, "w");
    if (!amFile) {
        PRINT_ERROR_MSG(RC_E_FAILED_TO_OPEN_FILE);
        PRINT_MSG_STR(cwd);
        PRINT_MSG_STR(amFileName);
        closeFiles(asFile, amFile);
        return RC_E_FAILED_TO_OPEN_FILE;
    }

    max_lines = getMaxMacroLines(asFile);

    macro_table = initializeMacro(max_lines);

    rc = scanMacros(asFile, macro_table);

    if (rc != 0) {
        PRINT_ERROR_MSG(RC_E_FAILED_TO_SCAN_MACROS);
        closeFiles(asFile, amFile);
        return RC_E_FAILED_TO_SCAN_MACROS;
    }

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

    closeFiles(asFile, amFile);

    return RC_OK;
}
