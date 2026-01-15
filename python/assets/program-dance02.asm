; ALU shimmer loop
; Visuals:
; - A runs and jitters
; - B periodically captures A (so B LEDs “follow” but lag)
; - RAM[1] stores a constant (used for ADD/SUB)
; - ALU is constantly active
LDI 0x7
STA 0x1        ; RAM[1] = 7  (constant)
LOOP:
    ADI 0x1        ; A++
    OUT
    MOVB_A         ; B = A       (B tracks A)
    ADD 0x1        ; A = A + 7   (big jump, ALU lights)
    SUB 0x1        ; A = A - 7   (back, but flags toggle)
    ADI 0x2        ; A += 2
    OUT
    JMP LOOP
