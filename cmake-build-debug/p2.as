; file p1.as
.entry LIST
.extern W
MAIN:   add r3,LIST
LOOP:   prn #48
        lea W , r6
        inc r6
        mov r3, K
        sub r1 , r4
        bne END
        cmp K, #-6
        bne &END
        dec r8
.entry MAIN
        jmp L3
        add L3, L3
END:    stop

; DATA SEGMENT
STR:    .string "abcd"
LIST:   .data 6,-9
        .data -100
K:      .data 31
.extern L3