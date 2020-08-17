; this is my assembly file

STR: .string "Hello World!"
MAIN:    prn STR
         cmp #1 , #2
         bne &MAIN
MAIN: prn #2

prn STR
NEWLABEL1: lea STR , r6
NUM: .data  5 ,   7
NUM2: .data 3, 1, 4
TEXT .string "missing colon"
.extern STR
