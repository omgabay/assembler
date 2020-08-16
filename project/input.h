#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
int validateNumber(char *num, int line);
int validateLabel(char * l, int line);
int isRegister(char * operand);
int extractOperand(char * op, int line);
int checkReserved(char *);
char *reserved[] = {"add","bne", "clr", "cmp", "dec","inc","jmp","jsr","lea","mov","not","prn","red","rts","stop","sub",".data",".extern",".entry",".string","r0","r1","r2","r3","r4","r5","r6","r7"};
