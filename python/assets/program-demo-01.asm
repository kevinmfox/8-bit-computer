; Fibonacci sequence

; Setup
LDI 0x1         ; Set A = 1 initially
STA 0xE         ; Store A = 1 into 0xE (Previous value)
STA 0xF         ; Store A = 1 into 0xF (Current value)

; Main loop
LOOP:
    JC  DONE    ; If Carry Flag is set, be done
    LDA 0xF     ; Load current value into A
    STA 0xD     ; Put that into temp storage
    ADD 0xE     ; Add our previous value
    OUT         ; Show it on the Display
    STA 0xF     ; Store our current value
    LDA 0xD     ; Load our temporary value
    STA 0xE     ; Store that into our previous value
    JMP LOOP    ; Keep on going

DONE:
    HLT
