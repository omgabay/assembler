#include <stdio.h>
#include <stdlib.h>
#include "objects.h"
int parsingDataInstruction(char *pos, int line);
int parsingStringInstruction(char *pos, int line);
void writeStringToMemory(char *string);
int doesLabelExist(char *name, int line, char *message);
labelEntry *addLabelToTable(char *name, int address);
CommandCode *getOpcode(char *operation);
NumericOperand *createNumericOperand(int num);
LabelOperand *createLabelOperand();
void createBinaryFile(char * filename);
void writeCodeMapToFile(FILE *file);
void writeDataMapToFile(FILE *file);
static void clean();
void deleteSymbolTable();
void deleteCodeMap();


/* secondPass function prototypes used in firstPass.c */
void saveRecord(char *labelName, int address, int line);
void saveEntry(char *label, int line);
void secondPass(int ICF, char *filename);
void deleteEntries();
void deleteLabelRecords();

CommandCode commands[] = {{"mov",0,0,2,3,0,3,2},{"cmp",1,0,3,3,0,3,2},
                          {"add",2,1,2,3,0,3,2},{"sub",2,2,2,3,0,3,2},{"lea",4,0,0,3,0,1,2},{"clr",5,1,0,1,0,1,1},
                          {"not",5,2,0,1,0,1,1},{"inc", 5,3,0,1,0,1,1},{"dec", 5,4,0,1,0,1,1},
                          {"jmp", 9,1,0,1,1,0,1}, {"bne",9,2,0,1,1,0,1},{"jsr",9,3,0,1,1,0,1},
                          {"red",12,0,0,1,0,1,1}, {"prn",13,0,1,1,0,1,1},{"rts",14,0,0,0,0,0,0},{"stop",15,0,0,0,0,0,0}
};