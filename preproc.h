
typedef struct Macro {
    int line_count;
    char macro_name [MAX_TOKEN_LENGTH];
    char * macro_contents;
}Macro;

char * extractMacroName(char* line);
int scanMacros (FILE * asFile, Macro * macro_table);
Macro * searchMacroName (Macro * macro_table, char * name);
int preasm (char * asFileName, char * amFileName);
