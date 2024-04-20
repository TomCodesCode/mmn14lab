#include "datastruct.h"

Instructions inst_prop[INST_SET_SIZE] = {
    {"mov",	0,	"123",	"0123"},
    {"cmp",	1,	"0123",	"0123"},
    {"add",	2,	"123",	"0123"},
    {"sub",	3,	"123",	"0123"},
    {"not",	4,	"123",	 NULL},
    {"clr",	5,	"123",	 NULL},
    {"lea",	6,	"123",	"12"},
    {"inc",	7,	"123",	 NULL},
    {"dec",	8,	"123",	 NULL},
    {"jmp",	9,	"13",	 NULL},
    {"bne",	10,	"13",	 NULL},
    {"red",	11,	"123",	 NULL},
    {"prn",	12,	"0123",	 NULL},
    {"jsr",	13,	"13",	 NULL},
    {"rts",	14,	 NULL,	 NULL},
    {"hlt",	15,	 NULL,	 NULL}
};

char * getInstByIdx(int idx) {
    if (idx >= 0 && idx < INST_SET_SIZE)
        return inst_prop[idx].inst;
        
    return NULL;
}

int numValidInstOperands(int inst) {
    if (inst_prop[inst].dest && inst_prop[inst].src)
        return 2;

    if (inst_prop[inst].dest)
        return 1;

    return 0;
}
