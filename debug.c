/*
 * used for debug purposes only
 * if required, it should be activated by adding -DDEBUG in Makefile
 */
#include "lib.h"

void debugPrints(void) {
#ifdef DEBUG
    dumpSymbolTbl();
    dumpOpcodesTbl();

    printf("IC: %d DC: %d PC %d #instr:%d #data:%d \n",
        IC_START, DC_START, PC, 
        DC_START?(DC_START-IC_START):(PC-IC_START), 
        DC_START?(PC-DC_START):0);
#endif
}