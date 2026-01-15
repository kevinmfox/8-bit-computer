; Dual-chase display loop
; Visuals:
; - A counts upward
; - B shows (0xFF - A), so it "counts downward"
; - RAM[0] mirrors A, RAM[1] mirrors B (RAM+MAR animate)
; - ALU active every step
INIT:
    LDI 0x1
    STA 0xF         ; RAM[0xF] = 1 (increment constant)
    LDI 0x0         ; Load 0 into A
LOOP:
    STA 0x0         ; RAM[0] = A          (show A on RAM)
    MOVB_A          ; B = A               (copy A so B changes too)
    LDI 0x0         ;
    SUB 0x0         ; A = 0 - RAM[0] = (-A)   (two's complement-ish step)
    SUB 0xF         ; A = A - 1 = (~A)        (because (-A) - 1 == ~A)
    MOVB_A          ; B = A               (B = ~original A)
    STA 0x1         ; RAM[1] = B          (show B on RAM)
    LDA 0x0         ; A = original A back
    ADD 0xF         ; A = A + 1           (increment)
    OUT             ; Put A on the Display
JMP LOOP