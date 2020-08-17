; Disclaimer- This file contains errors - Omer Gabay
.extern EXTERNAL
X: .data 3
Y: .data 5, 2
HELLO: .string "Hello World!"


; LABEL  ERRORS
WAY_WAY_TOO_LONG_LABEL_DEFINITION1: dec X
LAB1 : sub r1,r4
.extern WAY_WAY_TOO_LONG_LABEL_DEFINITION2
.entry L@bel
LABELEMPTYLINE:
TEXT .string "missing colon"


; REUSE OF LABELS
X: cmp #1,#1
.extern HELLO

; Misuse of addressing modes
prn &HELLO
lea #5 , HELLO
add #1 , #2
inc #1
sub r1, #1



;wrong syntax

STR: .string 123
STR2: .string "123
STR3: .string 123"
STR4: .string "a" "b"
.data "hello"
.data 3.14
.data 1 2 3
.data 1 , 5 ,
sub r1+r2,r3
add X Y
add X Y,

; wrong number of operands
clr
inc r3,r2
stop END
bne &END , STR
mov
sub HELLO


END: stop