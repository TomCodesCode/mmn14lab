#include "lib.h"

extern int IC_START;
extern int DC_START;
extern int PC;
extern int DC;

/*
 * Instruction properties functions
 */
int numValidInstOperands(int inst);
char * getInstByCode(int idx);
int isValidOperand(enum InstructionType inst_type, enum OperandType op_type, int operand_index);


/*
 * symbol table function
 */
int addSymbolVal(char * symbol, int symbol_type, int value);
int getSymbolTypeForARE(const char * symbol);
int getSymbolVal(const char * symbol, enum SymbolContext type, int ic, int * value);
int updateSymbolVal(char * symbol, int symbol_type, int value);
SymbolsTbl * getSymbolsTbl(void);
int dumpSymbolTbl(void);
int getNumOfSymbols();

/*
 * Opcode functions
 */
int calcOpcodePart(Opcodes * opcode, int wordtype, int num, int in_line_part);
int opcodePerOperand(AST *ast, int num, int in_line_part, int operand_idx);
int dumpOpcodesTbl(void);
int addOpcode(int wordtype, int num, enum Bool inc_line);
int getOpcodeTypeByOperand(enum OperandType op_type);
Opcodes * getOpcodes(void);
int getNumOfOpcodes();


/*
 * Debug functions
 */
void debugPrints(void);

/*
 * Memory manipulation
 */
char *my_strdup(const char *src, int delta);
#define strcatMalloc(a, b) a = strcat(strcpy(malloc(strlen(a) + strlen(b) + 1), a), b)

