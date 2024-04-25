#include "lib.h"

extern int IC_START;
extern int DC_START;
extern int PC;
extern int DC;

#define strcatMalloc(a, b) a = strcat(strcpy(malloc(strlen(a) + strlen(b) + 1), a), b)

int numValidInstOperands(int inst);
char * getInstByIdx(int idx);
int addSymbolVal(char * symbol, int symbol_type, int value);
int getSymbolTypeForARE(const char * symbol);
int getSymbolVal(const char * symbol, enum SymbolContext type, int ic, int * value);
int updateSymbolVal(char * symbol, int symbol_type, int value);
SymbolsTbl * getSymbolsTbl(void);
int dumpSymbolTbl(void);
int calcOpcodePart(Opcodes * opcode, int wordtype, int num, int in_line_part);
int opcodePerOperand(AST *ast, int num, int in_line_part, int operand_idx);
int middlePass(AST * code_ast, AST * data_ast);
int dumpOpcodesTbl(void);
int addOpcode(int wordtype, int num, enum Bool inc_line);
int getOpcodeTypeByOperand(enum OperandType op_type);
Opcodes * getOpcodes(void);
char *my_strdup(const char *src, int delta);
int isValidOperand(enum InstructionType inst_type, enum OperandType op_type, int operand_index);
int backendPass(char * filename);
int getNumOfSymbols();
int getNumOfOpcodes();

/* Debug functions */
void debugPrints(void) {
