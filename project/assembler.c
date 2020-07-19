#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


void firstPass(char *srcFile,char *outputFileName, int debug);



int main(int argc, char **argv){

    int i=1,debug;
    char c;
    printf("Do you want to run in debug? Extra print statements will be added [y:-]\n");
    scanf("%c", &c);
    debug = (tolower(c) == 'y') ? 1:0;
    while(--argc){
        char *name = argv[i++];

        /* Appending .as extension */
        char *src = (char *) malloc(strlen(name)+4);
        strcpy(src,name);
        strcat(src,".as");
        printf("-------------------------%s----------------------\n",src);
        printf("looking for file %s..\n", src);
        firstPass(src,name,debug);
        free(src);
    }

    return 0;
}

