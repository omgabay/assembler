#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "objects.h"
extern Word code[];
extern labelEntry *labels;
extern int debugMode;
FILE *externalFile;
FILE *entryFile;
void updateSymbolTable(int icf);
int setEntries();
FILE *createEntryFile(char *filename);
FILE *createExternalFile(char *filename);
void createBinaryFile(char *filename);
static void clean(char *filename);
void deleteEntries();
void deleteLabelRecords();
typedef struct entryRecord{
    char *name;
    int line;
    struct entryRecord *next;
} entryRecord;

typedef struct LabelRecord{
    char *name; /* name of label operand*/
    int address; /* where in code memory the label is used */
    int line;    /* line in source file */
    struct LabelRecord *next;
} LabelRecord;

LabelRecord *records = NULL;
LabelRecord *lastRecord = NULL;
entryRecord *entries =NULL;
int errorCnt2ndPass = 0;



void secondPass(int ICF,char *filename){
    updateSymbolTable(ICF);

    if(entries && !(errorCnt2ndPass = setEntries())){
        entryFile = createEntryFile(filename);
    }

    LabelRecord *lr = records;


    while(lr!=NULL){
        labelEntry *le = getLabelByName(lr->name);
        if(le == NULL){
            printf("Error: Label %s used on line %d was not found\n",lr->name, lr->line);
            errorCnt2ndPass++;
        }
        else{
           unsigned int ic = lr->address;
           Word word = code[ic];
           // printf("label %s on line %d saved on address %d\n", le->name, lr->line, le->address);

            if(word.type == LO){
                if(le->visibility == EXTERN){
                    word.w.lo->R=0;
                    word.w.lo->E=1;
                    /*
                     * Writing to .ext file the location in code memory where external label was used
                     */
                    if(!externalFile){
                        externalFile = createExternalFile(filename);
                    }
                    fprintf(externalFile,"%s %07d\n",le->name,lr->address);
                }else{
                    word.w.lo->address = le->address; /* writing to code the address of label */
                }
            }
           else if(word.type == JO){
               if(le->type != CODE) {
                   printf("label %s used on line %d must be referring code in a jump command\n", lr->name, lr->line);
                   errorCnt2ndPass++;
               }else if(le->visibility == EXTERN){
                   printf("Cannot jump to External label- '&%s' used on line %d\n", lr->name, lr->line);
                   errorCnt2ndPass++;
               }
               else{
                   word.w.no->data = le->address-ic+1;
               }
           }
        }
        lr = lr->next;
    }

    if(externalFile){
        fclose(externalFile);
    }

    if(errorCnt2ndPass == 0){
        createBinaryFile(filename);
    }else{
        printf("Failed at 2nd Pass! You have %d errors to fix\n",errorCnt2ndPass);
    }
    clean(filename);
}

void saveRecord(char *labelName, int address, int line){
    LabelRecord *record = (LabelRecord *) malloc(sizeof(LabelRecord));
    if(!labelName)
    {
        printf("ERROR! saveRecord received NULL\n");
        return;
    }
    record->name = (char *) malloc(strlen(labelName)+1);
    strcpy(record->name,labelName);
    record->address = address;
    record->line = line;
    /* Adding record when list records is empty */
    if(records == NULL){
        records = record;
        lastRecord = record;
    }
        /* Adding record to a non-empty list of records */
    else{
        lastRecord->next = record;
        lastRecord = lastRecord->next;
    }
}

void saveEntry(char *label, int line){
    static entryRecord *lastEntry = NULL;
    entryRecord *entry;
    if(!label){
        printf("ERROR! saveEntry received NULL\n");
        return;
    }
    entry = (entryRecord *) malloc (sizeof(entryRecord));
    if(!entry){
        printf("Error: Allocation Error on saveEntry\n");
        return;
    }
    entry->name = (char *) malloc(strlen(label)+1);
    strcpy(entry->name,label);
    entry->line = line;

    /* Adding entry to an empty list */
    if(entries == NULL){
        entries = entry;
        lastEntry = entry;
    }
        /* Adding entry to a non-empty list */
    else{
        lastEntry->next = entry;
        lastEntry = lastEntry->next;
    }
}

void updateSymbolTable(int icf){
    labelEntry *le = labels;
    while(le!=NULL){
        if(le->type == DATA && le->visibility != EXTERN)
        {
            le->address+=icf;
        }
        le = le->next;
    }
}

int setEntries(){
    entryRecord *entry = entries;
    int err = 0; /* Number of errors encountered in Entry instructions */
    while(entry != NULL){
       labelEntry *label = getLabelByName(entry->name);
       if(!label){
           printf("Error on line %d cannot create entry because label '%s' was not defined\n",entry->line, entry->name);
           err++;
       }
       else if(label->visibility == EXTERN){
           printf("Error on line %d - cannot create Entry - label '%s' must be defined in file\n",entry->line, entry->name);
           err++;
       }else{
           label->visibility = ENTRY;
           if(debugMode)
              printf("label %s is set to Entry\n", label->name);
       }
       entry = entry->next;
    }
    return err;
}

FILE *createEntryFile(char *filename){
    FILE *entFile;
    char *fname = (char *) malloc(strlen(filename)+5);
    strcpy(fname,filename);
    strcat(fname,".ent");
    entFile = fopen(fname,"w");
    free(fname);
    if(!entFile){
        printf("Error creating Entry file \"%s\"\n",filename);
        return NULL;
    }
    labelEntry *le = labels;
    while(le!=NULL){
        if(le->visibility == ENTRY)
        {
            fprintf(entFile,"%s %07d\n",le->name,le->address);
        }
        le = le->next;
    }
    fclose(entFile);
    return entFile;
}
FILE *createExternalFile(char *filename){
    char *fname = (char *) malloc(strlen(filename)+5);
    strcpy(fname,filename);
    strcat(fname,".ext");
    FILE *ext = fopen(fname, "w");
    free(fname);
    return ext;
}

static void clean(char *filename){
    printf("cleaning %s 2nd pass\n",filename);

    deleteLabelRecords();  /* Deleting Label Records */
    deleteEntries();    /* Deleting Entry Records */


    if(errorCnt2ndPass){
        if(entryFile){
            size_t len = strlen(filename)+5;
            char ent[len];
            strcpy(ent,filename);
            strcat(ent,".ent");
            int status = remove(ent);
            if(status == 0 && debugMode)
                printf("file %s was deleted\n", ent);
        }
        if(externalFile){
            size_t len = strlen(filename)+5;
            char ext[len];
            strcpy(ext,filename);
            strcat(ext,".ext");
            int status = remove(ext);
            if(status == 0 && debugMode)
                printf("file %s was deleted\n", ext);
        }
    }

}
void deleteEntries(){
    /* Deleting Entry Records */
    entryRecord *entry = entries;
    entries = NULL;
    while(entry){
        entryRecord *tmp = entry;
        entry = entry->next;
        free(tmp->name);
        free(tmp);
    }
}

void deleteLabelRecords(){
    /* Deleting Label Records */
    LabelRecord *lr = records;
    records = NULL;
    while(lr){
        LabelRecord *tmp = lr;
        lr = lr->next;
        free(tmp->name);
        free(tmp);
    }
}
