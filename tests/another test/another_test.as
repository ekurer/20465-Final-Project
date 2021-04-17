.entry K
.entry LOOP
MAIN: mov r0,r1
    cmp #-5,K
    cmp #2,S
    not K
LOOP:   jmp K
    prn K
    inc r7
    jsr S
END: stop
K: .data -5
S: .data 2
