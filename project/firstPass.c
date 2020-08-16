
#include "firstPass.h"
#define COMMAND_LENGTH 200
#define LOAD_ADDRESS 100
#define CODE_MAX_SIZE 1000
#define DATA_SIZE 1000

Word code[CODE_MAX_SIZE];
DataWord data[DATA_SIZE];
labelEntry *labels; /* Symbol Table to which all labels will be added */
int IC = LOAD_ADDRESS; /* Instruction Counter */
int ICF = LOAD_ADDRESS;
int DC = 0;  /* Data Counter */
int DCF = 0;
int debugMode = 0;

/*
 * firstPass will start translating the file to binary and create the symbol table which will later be used on the second pass.
 * firstPass will open and read the file given by filename parameter
 */
void firstPass(char *filename,char *outputFileName, int debug){
    int line = 0;
    int errorCount = 0;
    char command[COMMAND_LENGTH];

    debugMode = debug;
    FILE *srcFile = fopen(filename,"r");
    if(!srcFile){
        printf("Error: can't open file '%s'\nMake sure the file exists\n",filename);
        return;
    }


    while(memset(command,'\0',COMMAND_LENGTH) && fgets(command,COMMAND_LENGTH,srcFile) != NULL){
        char *tmp = (char *) malloc(strlen(command)+1); // DELETE
        strcpy(tmp,command); // DELETE
        int hasLabel = 0;  /* set to True if line has label definition  */
        char *labelName = NULL;
        char *pos = command;  /* pos is a pointer which we'll progress as we parse the command */
        line++;

        if(command[0] == ';') {
            /* line begins with ; indicates comment, therefore we skip to the next line */
            continue;
        }
        /* Label definition ends with colon - checking if ':' exists */
        if(strchr(command, ':'))
        {
            labelName = getLabel(&pos,line);
            if(labelName== NULL || doesLabelExist(labelName, line, NULL)){
                errorCount++;
                continue;
            }
            else{
                hasLabel = 1;
            }

        }

        char *operation = strtok(pos, " \n\t");
        if(!operation){
            if(!hasLabel && debugMode)
            {
                printf("Line %d is Empty\n", line);
            }
            else if(hasLabel){
                printf("Error on line %d: label can't be defined on an empty line\n",line);
                errorCount++;
            }
            continue;
        }

        pos = operation+strlen(operation)+1;

        if(strcmp(operation,".data") == 0){
            int address = DC;

            if(parsingDataInstruction(pos, line)){
                DC = address;
                errorCount++;
            }
            else if(hasLabel){
                labelEntry *le = addLabelToTable(labelName,address);   /* Label is added to the symbol table with the address of the first number */
                le->type = DATA;
            }
        }

        else if(strcmp(operation, ".string") == 0){
            int address = DC;
            if(parsingStringInstruction(pos, line)){
                errorCount++;
            }else{
                if(hasLabel){
                    labelEntry *le = addLabelToTable(labelName,address); /* Label is added to the symbol table with address that points to start of the string */
                    le->type = DATA;
                }
            }
        }

        else if(strcmp(operation, ".extern") == 0){
           char *label;
           if(inputLabel(&label, pos, line)){
               errorCount++;
           }
           else if(!doesLabelExist(label, line, "Warning on line %d: .extern - label '%s' was already defined\n")){
               labelEntry *le = addLabelToTable(label, 0);
               le->visibility = EXTERN;
           }

        }

        else if(strcmp(operation, ".entry") == 0){
            char *label;
            if(inputLabel(&label,pos,line)){
                errorCount++;
            }else{
                /* Label is valid - Saving entry label for the second pass */
                saveEntry(label,line);
            }
        }

        else{
            InstructionWord *iw = (InstructionWord *) calloc(1,sizeof(InstructionWord));
            CommandCode *cc = getOpcode(operation);
            char *srcOperand = NULL;
            char *targetOperand = NULL;
            Word srcWord; /* srcWord will be used if the src operand requires additional word */
            srcWord.type = None;
            Word targtWord;  /* targtWord will be used if the target operand requires additional word */
            targtWord.type = None;
            if(cc == NULL){
                errorCount++;
                printf("Error on line %d: Unknown command name- '%s'\n",line,operation);
                continue;
            }
            iw->opcode = cc->opcode;
            iw->func = cc->func;

            if(cc->numOfOperands == 2){
                if(inputTypeA(cc, &srcOperand,&targetOperand,pos,line))
                {
                    errorCount++;
                    continue;
                }
            }
            else if(cc->numOfOperands == 1){
                if(inputTypeB(cc, &targetOperand, pos,line))
                {
                    errorCount++;
                    continue;
                }
            }else{
                /* In this case command has 0 operands - we check if the command was given unnecessary args */
                if(skipSpaces(&pos) != 1)
                {
                    errorCount++;
                    printf("Error on line %d: %s command expects no operands or extraneous text\n",line,operation);
                    continue;
                }
            }

            if(srcOperand){
                Type type = getType(srcOperand,line);  /* gets the type of the operand */
                switch(type){
                    case NUM:
                                if(cc->immediate >= (unsigned) 2){
                                    int num = atoi(++srcOperand);
                                    iw->srcAddrType = 0; /* Immidiate Addressing - mode 0 */
                                    NumericOperand *no = createNumericOperand(num);  /* aka immidiate operand */
                                    srcWord.type = NO;
                                    srcWord.w.no = no;
                                }else{
                                    printf("Error on line %d: %s command doesn't accept number(immidiate) as src operand\n",line,operation);
                                    errorCount++;
                                }
                                break;
                    case REGISTER:
                                if(cc->reg >= (unsigned) 2){
                                    unsigned num = (unsigned) atoi(++srcOperand);
                                    iw->srcAddrType = 3;       /*  Register Addressing - mode 3  */
                                    iw->srcReg = num;
                                }else{
                                    errorCount++;
                                    printf("Error on line %d: %s command doesn't allow register as source operand\n", line,cc->name);
                                }
                                break;
                    case LABEL:
                                if(cc->direct >= (unsigned) 2){
                                    LabelOperand *lo = createLabelOperand();
                                    srcWord.w.lo = lo;
                                    srcWord.type = LO;
                                    iw->srcAddrType = 1;
                                }else{
                                    errorCount++;
                                    printf("Error on line %d: %s does not accept label ('%s') as src operand\n", line,operation,srcOperand);
                                }
                                break;
                    case JUMP_LABEL:
                                if(cc->jump >= (unsigned) 2){
                                    NumericOperand *jmp = createNumericOperand(0); /* jump distance will be resolved on 2nd pass */
                                    srcWord.w.no = jmp;
                                    srcWord.type = JO;
                                    iw->srcAddrType = 2; /* Relational Addressing (jump) - mode 2 */
                                    srcOperand++; /* skips the '&' */
                                }else{
                                    errorCount++;
                                    printf("Error on line %d: %s does not accept jump-label ('%s') as src operand\n", line,operation,srcOperand);
                                }
                                break;

                    default:
                        printf("Error on line %d: src operand ('%s') is of unknown type\n", line ,srcOperand);
                        errorCount++;
                }
            }

            if(targetOperand){
                Type type = getType(targetOperand, line);
                switch(type){
                    case NUM:
                        if(cc->immediate %(unsigned)2 == 1){
                            int num = atoi(++targetOperand);
                            iw->dstAddrType = 0; /* Immidiate Addressing - mode 0 */
                            NumericOperand *no = createNumericOperand(num);  /* aka immidiate operand */
                            targtWord.type = NO;
                            targtWord.w.no = no;
                        }else{
                            printf("Error on line %d: %s command doesn't accept number(immidiate) as target operand\n",line,operation);
                            errorCount++;
                        }
                        break;

                    case REGISTER:
                        if(cc->reg % (unsigned) 2 == 1){
                            unsigned num = (unsigned) atoi(++targetOperand);
                            iw->dstAddrType = 3;       /* Register Addressing - mode 3  */
                            iw->dstReg = num;
                        }else{
                            errorCount++;
                            printf("Error on line %d: %s command doesn't allow register as target operand\n", line,cc->name);
                        }
                        break;

                    case LABEL:
                        if(cc->direct % (unsigned) 2 == 1){
                            LabelOperand *lo = createLabelOperand();
                            targtWord.w.lo = lo;
                            targtWord.type = LO;
                            iw->dstAddrType = 1;
                        }else{
                            errorCount++;
                            printf("Error on line %d: %s does not accept label ('%s') as target operand\n", line,operation,targetOperand);
                        }
                        break;
                    case JUMP_LABEL:
                        if(cc->jump % (unsigned) 2 == 1){
                            NumericOperand *jmp = createNumericOperand(0); /* jump distance will be resolved on 2nd pass */
                            targtWord.w.no = jmp;
                            targtWord.type = JO;
                            iw->dstAddrType = 2; /* Relational Addressing (jump) - mode 2 */
                            targetOperand++; /* skips the '&' */
                        }else{
                            errorCount++;
                            printf("Error on line %d: %s does not accept jump-label ('%s') as target operand\n", line,operation,targetOperand);
                        }
                        break;
                    default:
                        printf("Error on line %d: %s - target operand ('%s') is of unknown type\n", line,operation,targetOperand);
                        errorCount++;
                }
            }
            iw->A = 1;
            iw->R = 0;
            iw->E = 0;
            code[IC].type = IW;   /* setting code word to type Instruction */
            code[IC].w.iw = iw;

             if(hasLabel){
                 /* Adding label to symbol table with IC as the address of the label */
                 labelEntry *le = addLabelToTable(labelName,IC);
                 le->type= CODE;
             }

             IC++;

             if(srcWord.type != None){
                 if(srcWord.type == JO || srcWord.type == LO){
                     saveRecord(srcOperand,IC,line);
                 }
                 code[IC++] = srcWord;
             }
            if(targtWord.type != None){
                if(targtWord.type == JO || targtWord.type == LO){
                    saveRecord(targetOperand,IC,line);
                }
                code[IC++] = targtWord;
            }
         }

     }

    fclose(srcFile);
    if(errorCount == 0){
        ICF = IC;
        DCF = DC;
        secondPass(ICF, outputFileName);
    }else{
        printf("\nYou have %d errors to fix\n", errorCount);
        deleteLabelRecords();
        deleteEntries();
    }

    clean();


 }
 /*
  * parsingDataInstruction gets the numbers given to the ".data instruction" and stores them at the data map
  * if syntax error is detected -1 is returned and the numbers written up until this point will be ignored
  */
int parsingDataInstruction(char *pos , int line){
    int status;
    if((status=skipSpaces(&pos)))
    {
        if(status == 1)
            printf("Error on line %d: expecting an argument after .data instruction\n", line);
        if(status == 2)
            printf("Error on line %d: invalid comma after .data instruction\n",line);
        return -1;
    }
    while(*pos != '\0'){
        if(getNumber(&pos,&data[DC],line)){
            return -1;
        }
        if(debugMode){
            printf("%d was saved to data map\n", data[DC].word);
        }
        DC++;
    }
    return 0;
}
/*
 * parsingStringInstruction parses .string instruction - checks for syntax error and if everything is okay writes the string to the data map
 */
int parsingStringInstruction(char *pos, int line){
    char *string = NULL;
    int status;
    if((status=skipSpaces(&pos)))
    {
        if(status == 1)
            printf("Error on line %d: expecting a string after .string instruction\n", line);
        if(status==2)
            printf("Error on line %d: invalid comma after .string instruction\n",line);
        return -1;
    }
    string = getString(pos,line);
    if(!string){
        return -1;
    }else{
        writeStringToMemory(string);
    }
    return 0;

}

void writeStringToMemory(char *string){
    char *p = string;

    while(*p != '\0'){
        if(debugMode)
            printf("'%c' was written to data map - ASCII in hex  %06x\n",*p,(unsigned int)*p);
        data[DC++].word = (int) *p++;
    }
    data[DC++].word = (unsigned int)'\0';
    if(debugMode)
        printf("'\\0' was written to data map - ASCII in hex  %06x\n",'\0');
}
/*
 * doesLabelExist checks if the label's name was already defined
 * if label was already defined -1 is returned, 0 otherwise
 */
int doesLabelExist(char *name, int line, char *message) {
    labelEntry *le = labels;
    if(name == NULL)
        return -1;
    while(le){
        if(strcmp(le->name, name)==0){
            if(!message)
                printf("Error on line %d: Label '%s' was already defined\n", line , name);
            else
                printf(message, line, name);
;            return -1;
        }
        le = le->next;
    }
    return 0;
}
labelEntry *addLabelToTable(char *name, int address) {
    static labelEntry *lastLabel = NULL; /* Points to the last label added to labels */
    labelEntry *lab = (labelEntry *) calloc(1,sizeof(labelEntry));
    lab->name = (char *) malloc(strlen(name)+1);
    strcpy(lab->name,name);
    lab->address = address;
    lab->next = NULL;
    lab->visibility = LOCAL;
    if(labels == NULL){
        labels = lab;
        lastLabel = lab;
    }else{
        lastLabel->next = lab;
        lastLabel = lastLabel->next;
    }
    if(debugMode){
        printf("label '%s' was added to the Symbol Table\n",lab->name);
    }
    return lab;
}
labelEntry *getLabelByName(char *name) {
    labelEntry *le = labels;
    if(name == NULL)
        return NULL;
    while(le){
        if(strcmp(le->name, name)==0){
            return le;
        }
        le = le->next;
    }
    return NULL;
}
CommandCode *getOpcode(char *s){
    int i;
    if(s == NULL)
        return NULL;
    size_t length = sizeof(commands)/sizeof(commands[0]);

    for(i=0; i<length; i++){
        if(strcmp(s,commands[i].name)==0)
            return &commands[i];
    }
    return NULL;
}
NumericOperand *createNumericOperand(int num){
    NumericOperand *no = (NumericOperand *) malloc(sizeof(NumericOperand));
    no->data = num;
    no->A=1;
    no->R=0;
    no->E=0;
    return no;
}
LabelOperand *createLabelOperand(){
    LabelOperand *lo = (LabelOperand *) malloc(sizeof(LabelOperand));
    lo->address = 0;  /* label's address will be resolved on the 2nd pass */
    lo->A = 0;
    lo->R = 1;
    lo->E = 0;
    return lo;
}

void createBinaryFile(char *filename){
    /* Adding .ob extension to filename */
    char *fname = (char *) malloc(strlen(filename)+4);
    strcpy(fname,filename);
    strcat(fname,".ob");
    FILE *binaryFile = fopen(fname, "w");
    fprintf(binaryFile, "\t %d %d\n", ICF-100,DCF);
    writeCodeMapToFile(binaryFile);
    writeDataMapToFile(binaryFile);
    fclose(binaryFile);
    printf("%s created successfully\n", fname);
    free(fname);
}

void writeCodeMapToFile(FILE *output){
    int24 res;
    int i;
    for(i=LOAD_ADDRESS;i<IC;i++){
        if(code[i].type == IW){
            InstructionWord *iw = code[i].w.iw;
            res.word = iw->opcode;
            res.word <<= (unsigned)2;
            res.word += iw->srcAddrType;
            res.word <<= (unsigned)3;
            res.word += iw->srcReg;
            res.word <<= (unsigned)2;
            res.word += iw->dstAddrType;
            res.word <<= (unsigned)3;
            res.word += iw->dstReg;
            res.word <<= (unsigned)5;
            res.word += iw->func;
            res.word <<= (unsigned)1;
            res.word +=iw->A;
            res.word <<= (unsigned)2;

        }else if(code[i].type == NO || code[i].type == JO){
            NumericOperand *no = code[i].w.no;
            res.word = no->data;
            res.word <<= (unsigned) 1;
            res.word += (unsigned) no->A;
            res.word <<= (unsigned)2;
        }else if(code[i].type == LO){
            LabelOperand *lo = code[i].w.lo;
            res.word = lo->address;
            res.word <<= (unsigned) 2;
            res.word += lo->R;
            res.word <<= (unsigned) 1;
            res.word += lo->E;
        }
        else if(code[i].type == None){
            printf("ERROR UNKNOWN CODE-WORD\n");
            return; // ERROR
        }
        fprintf(output,"%07d\t%06x\n", i,res.word);
    }
}

void writeDataMapToFile(FILE *output){
    int i;
    int24 tmp;
    for(i=0; i<DCF; i++) {
        if(i>0)
            fprintf(output,"\n");
        tmp.word = data[i].word;
        fprintf(output,"%07d %06x", i + ICF, tmp.word);
    }
}

static void clean(){
    printf("cleaning\n");
    deleteCodeMap();
    IC = LOAD_ADDRESS;
    DC = DCF = 0;
    ICF = LOAD_ADDRESS;
    deleteSymbolTable();
}
void deleteCodeMap(){
  int i;
  for(i=LOAD_ADDRESS; i<IC; i++){
      Word word = code[i];
      switch(word.type){
          case IW:
              free(word.w.iw);
              break;
          case NO: case JO:
               free(word.w.no);
               break;
          case LO:
              free(word.w.lo);
              break;
          default:
              printf("deleteCodeMap - None\n");
              break;
      }
      word.type = None;
  }

}
void deleteSymbolTable(){
    labelEntry *le = labels;
    labels = NULL;
    while(le != NULL){
        labelEntry *tmp = le;
        le = le->next;
        free(tmp->name);
        free(tmp);
    }
}