
#include "secondPass.h"
extern Word code[];
extern labelEntry *labels;
extern int debugMode;
FILE *externalFile;
FILE *entryFile;



LabelRecord *records = NULL;

entryRecord *entries =NULL;
int errorCnt2ndPass = 0;



void secondPass(int ICF,char *filename){
    LabelRecord *record =records; /* record is used to complete the words in code memory which were left without address */
    updateSymbolTable(ICF);

    if(entries && !(errorCnt2ndPass = setEntries())){
        entryFile = createEntryFile(filename);
    }




    while(record!=NULL){
        labelEntry *label = getLabelByName(record->name);

        if(label == NULL){
            printf("Error on line %d: Label %s was not found\n", record->line , record->name);
            errorCnt2ndPass++;
        }
        else{
           unsigned int ic = record->address; /* ic refers to the word in code memory that needs completion*/
           Word word = code[ic];

            if(word.type == LO){
                if(label->visibility == EXTERN){
                    word.w.lo->R=0;
                    word.w.lo->E=1;
                    /*
                     * Writing to .ext file the location in code memory where external label was used
                     */
                    if(!externalFile){
                        externalFile = createExternalFile(filename);
                    }
                    fprintf(externalFile,"%s %07d\n",label->name, ic);
                }else{
                    word.w.lo->address = label->address; /* writing to code the address of label */
                }
            }
           else if(word.type == JO){
               if(label->type != CODE) {
                   printf("Error on line %d: Label '%s' must be referring code in a jump command\n", record->line, record->name);
                   errorCnt2ndPass++;
               }else if(label->visibility == EXTERN){
                   printf("Error on line %d: Cannot jump to External label- '&%s' used on line %d\n", record->line, record->name, record->line);
                   errorCnt2ndPass++;
               }
               else{
                   word.w.no->data = label->address-ic+1;
               }
           }
        }
        record = record->next;
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
    static LabelRecord *lastRecord = NULL;
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
    record->next = NULL;
    /* Adding record when list records is empty */
    if(records == NULL){
        records = record;
        lastRecord = record;
    }
        /* Adding record to a non-empty list of records */
    else{
        lastRecord->next = record;
        lastRecord = record;
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
    entry->next = NULL;

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
           printf("Error on line %d: cannot create entry because label '%s' was not defined\n",entry->line, entry->name);
           err++;
       }
       else if(label->visibility == EXTERN){
           printf("Error on line %d: cannot create Entry - label '%s' must be defined in file(external not allowed)\n",entry->line, entry->name);
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
    labelEntry *label;
    char *fname = (char *) malloc(strlen(filename)+5);
    strcpy(fname,filename);
    strcat(fname,".ent");
    entFile = fopen(fname,"w");

    if(!entFile){
        printf("Error creating Entry file \"%s\"\n",filename);
        return NULL;
    }
    label = labels;
    while(label!=NULL){
        if(label->visibility == ENTRY)
        {
            fprintf(entFile,"%s %07d\n",label->name,label->address);
        }
        label = label->next;
    }
    fclose(entFile);
    if (debugMode)
        printf("%s created successfully\n", fname);
    free(fname);
    return entFile;
}
FILE *createExternalFile(char *filename){
    FILE *ext;
    char *fname = (char *) malloc(strlen(filename)+5);
    strcpy(fname,filename);
    strcat(fname,".ext");
    ext = fopen(fname, "w");
    if(debugMode)
        printf("%s created successfully\n", fname);
    free(fname);
    return ext;
}

static void clean(char *filename){
    printf("cleaning %s 2nd pass\n",filename);

    deleteLabelRecords();  /* Deleting Label Records */
    deleteEntries();    /* Deleting Entry Records */
    entryFile = NULL;
    externalFile = NULL;

    if(errorCnt2ndPass){
        if(entryFile){
            int status;  /* status for remove operation */
            char *ent;
            size_t len = strlen(filename)+5;
            ent = (char *) calloc(len,sizeof(char));
            strcpy(ent,filename);
            strcat(ent,".ent");
            status = remove(ent);
            if(status == 0 && debugMode)
                printf("file %s was deleted\n", ent);
            free(ent);
        }
        if(externalFile){
            int status;  /* status for remove operation */
            char *ext;
            size_t len = strlen(filename)+5;
            ext = (char *) calloc(len,sizeof(char));
            strcpy(ext,filename);
            strcat(ext,".ext");
            status = remove(ext);
            if(status == 0 && debugMode)
                printf("file %s was deleted\n", ext);
            free(ext);
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
