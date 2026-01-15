; Register transfer & ADD test
; Will test: NOP, LDI, STA, ADD, MOVB_A, MOVA_B, OUT, HLT
; PASS output: 0x0C
NOP             ; do nothing
LDI 0x1         ; A = 1
STA 0x0F        ; RAM[0x0F] = 1
LDI 0x0B        ; A = 0x0B
ADD 0x0F        ; A = 0x0C        (tests ADD-from-RAM)
MOVB_A          ; B = 0x0C        (tests MOVB_A)
LDI 0x0         ; A = 0
MOVA_B          ; A = 0x0C        (tests MOVA_B)
OUT             ; expect 0x0C
HLT
