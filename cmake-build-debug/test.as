; this is a test file
INC:    inc r8
        add L3, L3
        .extern L3
        sub r1, &STR
        bne L3
STR:    .data "hello"
END:    stop END

