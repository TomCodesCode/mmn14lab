#define MAX_RAM_SIZE 4096
#define MAX_LINE_LENGTH 81
#define MAX_TOKEN_LENGTH 31
#define OPCODE_COUNT 16
#define ARE_BITS 2
#define MAX_LABELS 100
#define START_ADDRESS 100
#define MAX_COMMAND_SIZE 10
#define INST_SET_SIZE 16
#define REG_NAME_LEN 2
#define MAX_DATA_OBJECTS 50
#define MAX_FILENAME_LCHK 128
#define MAX_FILENAME_LEN (MAX_FILENAME_LCHK + 8)
#define ENCRYPTED_OPCODE_LEN 7

#define FN_OB_EXT  ".ob"
#define FN_ENT_EXT ".ent"
#define FN_EXT_EXT ".ext"
#define FN_AM_EXT  ".am"

enum Bool {
    FALSE = 0,
    TRUE
} BOOL;
