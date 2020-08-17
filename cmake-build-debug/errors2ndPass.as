.entry Hello
DATA:   .extern X

; entry error - label doesnt exist
    .entry NOTFOUND
; entry error - cannot create entry of external label
    .entry X


cmp #12 , NOTFOUND
inc NOTFOUND

; x not found
bne x
; Correct
bne X


; Error relative-jump to external label is not allowed
jmp &X

bne &Numbers

add #1 , Numbers
prn Y

;Data for program :)
Y: .data 10
Numbers: .data 1,2,3,4,5
Hello: .string "Hello World"
