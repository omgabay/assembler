#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "objects.h"


void updateSymbolTable(int icf);
int setEntries();
FILE *createEntryFile(char *filename);
FILE *createExternalFile(char *filename);
void createBinaryFile(char *filename);
static void clean(char *filename);
void deleteEntries();
void deleteLabelRecords();
/* SecondPass objects */

/*
 * Object that records entry instructions for handling in 2nd pass
 */
typedef struct entryRecord{
    char *name; /* name of entry label */
    int line;   /* in which line appeared entry instruction */
    struct entryRecord *next;
} entryRecord;

/*
 *  object that records the use of a label in command for handling in 2nd pass
 */
typedef struct LabelRecord{
    char *name; /* name of label operand*/
    int address; /* where in code memory the label is used */
    int line;    /* line in source file */
    struct LabelRecord *next;
} LabelRecord;