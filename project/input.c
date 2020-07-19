#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "objects.h"
#include "input.h"
#define LABEL_LENGTH 31
int validateNumber(char *num);
int validateLabel(char * l);
int isRegister(char * operand);
int extractOperand(char * op);
char *getLabel(char **ptr, int line){
    char *pos = *ptr;
    char *label = strtok(pos, ":");
    pos = label + strlen(label) + 1;
    *ptr = pos;
    if(strlen(label) > LABEL_LENGTH){
        printf("Error on line %d: Label exceeds the permitted length\n",line);
        return NULL;
    }
    if(validateLabel(label)){
        return NULL;
    }
    if(checkReserved(label)){
        printf("Error on line %d: label '%s' is reserved word and cannot be used!",line,label);
        return NULL;
    }
    return label;
}


int checkReserved(char *label){
    int size = sizeof(reserved)/sizeof(reserved[0]);
    int i;
    for(i=0; i<size; i++){
        if(strcmp(label,reserved[i]) == 0){
            return -1;
        }
    }
    return 0;
}
int validateLabel(char *label) {
    char *l = label;
    if(!isalpha(*l)){
        printf("Error: First character of a label must be a letter\n");
        return -1;
    }

    while(*++l != '\0'){
        if(isspace(*l)){
            printf("Error: Label cannot contain spaces. Make sure you don't have space between the label's name and the colon\n");
            return -1;
        }
        if(!isdigit(*l)&&!isalpha(*l)){
            printf("Error: Label can contain only letters and digits\n");
            return -1;
        }
    }
        if(checkReserved(label)){
            printf("%s is reserved word and cant be used as a label\n", label);
            return -1;
        }

    return 0;
}

int skipSpaces(char **ptr) {
    char *pos = *ptr;
    while(isspace(*pos)){
        pos++;
    }
    if(*pos == '\0'){
        *ptr = pos;
        return 1; /*signal to the caller that we reached the end of the line */
    }
    if(*pos == ',')
    {
        return 2; /*  illegal comma detected  */
    }
    *ptr = pos;
    return 0;
}
int getNumber(char **ptr, DataWord *dw, int line){
   char *pos = *ptr;
   char *num;
   int notlast = 0;
   if(strchr(pos,',')){
       num = strtok(pos,",");
       notlast = 1;
   }
   else{
       num = strtok(pos,"\n");
   }
   if(!num) {
       return -1;
   }
   pos = num +strlen(num)+1;

    if(validateNumber(num)){
        return -1;
    }
    /* notlast is set to true (1) if after the number we read a comma meaning that we have at least one more number*/
    if(notlast){
        int status = skipSpaces(&pos);
        if(status == 1){
            printf("Error on line %d: .data - expecting a number after comma\n", line);
            return -1;
        }
        else if(status == 2){
            printf("Error on line %d: .data - consecutive commas after reading number\n", line);
            return -1;
        }
    }
    dw->word = atoi(num);
    *ptr = pos;
    return 0;
}

/*
 * validateNumber checks if the string represents a number- it can start with +/- sign and after that only digits expected
 * if there are spaces after the number they get removed
 */
int validateNumber(char *num){
    char *p = num;
    int flag = 0;
    if(*p == '+' || *p == '-')
        p++;
    if(!isdigit(*p))
    {
        printf("Expecting an integer - '%c' is not a digit\n", *p);
        return 1;
    }
    while(*p != '\0'){
        if(!flag){
            if(isdigit(*p)){
                p++;
            }

            else if(isspace(*p)){
                *p++ = '\0';
                flag = 1;
            }
            else{
                printf("Expecting an integer - %c is not allowed\n", *p);
                return 1;
            }
        }
        else{
            if(isspace(*p)){
                p++;
            }
            else{
                if(isdigit(*p) || *p == '+' || *p == '-'){
                    printf("Missing comma between numbers\n");
                }else{
                    printf("Extraneous text after number '%s' \n", num);
                }
                return 1;
            }
        }
    }
    return 0;
}
/*
 * inputTypeA handels input to instructions that have two operands - updates p1 and p2 with operands
 */
int inputTypeA(CommandCode *cc, char **p1, char **p2, char *pos, int line) {
    if(skipSpaces(&pos) == 2){
        printf("Error on line %d: %s command - Incorrect comma before operand\n", line , cc->name);
        return -1;
    }
    char *src = strtok(pos,",\n");
    if(src == NULL)
    {
        printf("Error on line %d: Command %s missing operands\n",line,cc->name);
        return -1;
    }
    pos = src+strlen(src)+1;
    char *target = strtok(pos , " \n");
    if(target == NULL)
    {
        printf("Error on line %d: Command %s missing target operand\n",line,cc->name);
        return -1;
    }
    pos = target+strlen(target)+1;
    if(extractOperand(src) || extractOperand(target)){
        return -1;
    }
    int i=skipSpaces(&pos);
    if(i!=1){
        printf("Error on line %d: %s - Extraneous text after the end of command\n",line, cc->name);
        return -1;
    }
    *p1 = src;
    *p2 = target;
    return 0;
}
/*
 * inputTypeB handels input to instructions that have one operands - updates ptr with operand
 */
int inputTypeB(CommandCode *cc, char **ptr,char *pos,int line){
    if(skipSpaces(&pos) == 2){
        printf("Error on line %d: %s command - Incorrect comma before operand\n", line , cc->name);
        return -1;
    }
    char *target = strtok(pos , " \n");
    if(target == NULL)
    {
        printf("Error on line %d: Command %s missing target operand\n",line,cc->name);
        return -1;
    }
    pos = target+strlen(target)+1;

    int i=skipSpaces(&pos);
    if(i!=1){
        printf("Error on line %d: %s - Extraneous text after the end of command\n",line, cc->name);
        return -1;
    }
    *ptr = target;
    return 0;
}



/*
 * inputLabel is used in .entry and .extern instructions to get the label operand and saved it in address given by ptr
 * inputLabel returns -1 at failure, 0 on success
 */
int inputLabel(char **ptr, char *pos, int line){
    if(skipSpaces(&pos) == 2){
        printf("Error on line %d: Invalid comma before label operand\n",line);
        return -1;
    }
    char *label = strtok(pos , " \n");
    if(label == NULL)
    {
        printf("Error on line %d: missing label operand\n", line);
        return -1;
    }
    pos = label+strlen(label)+1;

    int i=skipSpaces(&pos);
    if(i!=1){
        printf("Error on line %d: extraneous text after the end of command\n", line);
        return -1;
    }
    if(validateLabel(label))
    {
        return -1;
    }
    *ptr = label;
    return 0;
}
/*
 *  getType - returns the type of the operand based on the string provided
 *  types of operand can be - NUM , REGISTER, LABEL , JUMP_LABEL and UNDEFINED is returned at failure
 */
Type getType(char *operand){
    if(*operand == '#'){
        if(validateNumber(++operand)){
            return UNDEFINED;
        }
        return NUM;
    }else if(*operand == '&'){
        if(validateLabel(++operand)){
            return UNDEFINED;
        }
        return JUMP_LABEL;
    }else if(isRegister(operand)){
        return REGISTER;
    }else{
        if(validateLabel(operand)){
            return UNDEFINED;
        }
        return LABEL;
    }
}
/*
 *  isRegister checks if the given string represents one of the registers
 */
int isRegister(char *operand){
    if(*operand != 'r' || strlen(operand)!=2)
        return 0;
    operand++;
    if(*operand>='0' && *operand<='7')
        return 1;
    return 0;
}
int extractOperand(char *op){
    int flag = 0;  /* flag is set to 1 when we read the first space after an operand*/
    char *p = op;
    while(*p!='\0'){
        if(!flag){
            if(isspace(*p)){
                *p = '\0';
                flag = 1;
            }else if(*p == ','){
                printf("Incorrect comma -- extractOperand\n");
                return -1;
            }
        }else{
           if(!isspace(*p)) {
               printf("Comma expected after %s\n",op);
               return -1;
           }
        }
        p++;
    }
    return 0;
}


char *getString(char *pos, int line){
    int flag = 0;
    char *string;
    if(*pos != '"'){
        printf("Error on line %d: .string instruction expects a string and must include quotation mark \" \"\n", line);
        return NULL;
    }

    string = ++pos;

    while(*pos != '\0'){

        if(!flag){
            /*  flag - false -> the string was not ended yet  */
            if(*pos == '"'){
                *pos = '\0';
                flag = 1;
            }
        }
        else{
            if(!isspace(*pos)){
                printf("Error on line %d: Extraneous text after .string command\n",line);
                return NULL;
            }
        }
        pos++;
    }
    if(!flag){
        printf("Error on line %d: String was not terminated by end-quote\n",line);
        return NULL;
    }
    return string;
}

