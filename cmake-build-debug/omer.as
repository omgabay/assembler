; this is my assembly file

STR: .string "Hello World!"
MAIN:    prn STR
         cmp #1 , #2
         bne &MAIN

;MAIN: lea &STR , r6
;NUM: .data  5 , "hello"
;NUM2: .data 3 1 4