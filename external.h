#include "lib.h"

extern int IC;
extern int DC;

int numValidInstOperands(int inst);
char * getInstByIdx(int idx);
/*SymbolsTbl * getSymbolsTbl(void);*/
int addSymbolVal(char * symbol, int symbol_type, int value);
int getSymbolVal(const char * symbol, enum SymbolContext type, int ic, int * value);
int dumpSymbolTbl(void);
int calcOpcodePart(Opcodes * opcode, int wordtype, int num, int in_line_part);
int opcodePerOperand(AST *ast, int num, int in_line_part, int operand_idx);
int midPassing(AST *ast);
int dumpOpcodesTbl(void);
int addOpcode(int wordtype, int num, enum Bool inc_line);
int getOpcodeTypeByOperand(enum OperandType op_type);

