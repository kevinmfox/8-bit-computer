; Memory test
; Will test: LDI, STA, LDA, LDB, MOVA_B, OUT, HLT
; PASS output: 0x0B
LDI 0x9        ; A = 0x09 (signature value)
OUT            ; just to set it to an incorrect value
STA 0x0E       ; RAM[0x0E] = 0x09
LDI 0x3        ; A = 0x03 (signature value)
STA 0x0F       ; RAM[0x0F] = 0x03
LDA 0x0E       ; A = 0x09 (proves STA -> LDA round-trip)
ADI 0x2        ; A = 0x0B
LDB 0x0F       ; B = 0x03 (loads B from RAM)
MOVA_B         ; A = 0x03 (proves B truly received 0x03)
ADI 0x8        ; A = 0x0B
OUT            ; expect 0x0B
HLT