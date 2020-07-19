#include <stdio.h>
#include <stdlib.h>
#include "objects.h"
#include "secondPass.h"
int parsingDataInstruction(char *pos, int line);
int parsingStringInstruction(char *pos, int line);
void writeStringToMemory(char *string);
int doesLabelExist(char *name);
labelEntry *addLabelToTable(char *name, int address);
CommandCode *getOpcode(char *operation);
NumericOperand *createNumericOperand(int num);
LabelOperand *createLabelOperand();
void createBinaryFile(char * filename);
void writeCodeMapToFile(FILE *file);
void writeDataMapToFile(FILE *file);
static void clean();
void deleteSymbolTable();