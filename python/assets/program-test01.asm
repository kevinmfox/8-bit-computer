; Flags & Jumps test
; Will test: LDI, STA, SUB, ADI, JC, JZ, JMP, OUT, HLT
; PASS output: 0x0A
LDI 0x1     ; A = 1
STA 0xF     ; RAM[0xF] = 1
LDI 0x0     ; A = 0
SUB 0xF     ; A = 0 - 1 = 0xFF, CF=0, ZF=0
JC  0xD     ; must NOT jump (CF=0)
ADI 0x1     ; A = 0x00, CF=1, ZF=1
JZ  0x8     ; must jump (ZF=1)
JMP 0xD     ; FAIL if we get here
JC  0xA     ; must jump (CF=1)
JMP 0xD     ; FAIL if we get here
LDI 0xA     ; PASS code
OUT
HLT
LDI 0xE     ; FAIL code
OUT
HLT
