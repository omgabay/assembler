#include <string.h>

typedef struct {
    int word : 24;
} DataWord;

typedef struct {
    unsigned int word : 24;
} int24;

typedef struct {
    char *name;     /* command name */
    unsigned int opcode : 6;
    unsigned int func : 5;
    unsigned int immediate : 2;
    unsigned int direct : 2;
    unsigned int jump : 2;
    unsigned int reg : 2;
    int numOfOperands;
} CommandCode;

typedef struct  {
    unsigned int opcode : 6;
    unsigned int srcAddrType : 2;
    unsigned int srcReg : 3;
    unsigned int dstAddrType : 2;
    unsigned int dstReg : 3;
    unsigned int func : 5;
    unsigned int A : 1;
    unsigned int R : 1;
    unsigned int E : 1;
} InstructionWord;

typedef struct {
    unsigned address : 21;
    unsigned int A : 1;
    unsigned int R : 1;
    unsigned int E : 1;
}LabelOperand;

typedef struct {
    int data : 21;
    unsigned int A : 1;
    unsigned int R : 1;
    unsigned int E : 1;
} NumericOperand;

typedef struct{
    enum {None=-1,IW,NO,LO,JO} type;  /* IW - instruction | NO - Numeric operand(immediate) | LO - Label | JO - JUMP */
    union{
        InstructionWord *iw;
        NumericOperand *no;
        LabelOperand *lo;
    } w;
} Word;


typedef struct labelEntry{
    char *name;
    unsigned int address;
    enum {CODE,DATA} type;
    enum {LOCAL, EXTERN, ENTRY} visibility;
    struct labelEntry *next;
} labelEntry;

typedef enum{UNDEFINED=-1,NUM=0,LABEL,JUMP_LABEL,REGISTER} Type;


char *getLabel(char **, int line);
char *getString(char *, int line);
int getNumber(char ** pos, DataWord * word, int line);
int skipSpaces(char **ptr);
int inputLabel(char **ptr, char *pos, int line);
int inputTypeA(CommandCode *cc, char **p1, char **p2,char *pos, int line);
int inputTypeB(CommandCode *, char **, char *, int line);
Type getType(char *);
labelEntry *getLabelByName(char *name);