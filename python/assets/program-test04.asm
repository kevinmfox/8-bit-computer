; JMPV test
; Will test: LDI, STA, JMPV, OUT, HLT
; PASS output: 0x0D
LDI 0xC        ; A = 0x0C (address of PASS block)
STA 0x0F       ; RAM[0x0F] = 0x0C (jump vector)
LDI 0xE        ; preload FAIL code in A
JMPV 0x0F      ; PC = RAM[0x0F] (should jump to 0x0C)
OUT            ; if JMPV fails, we fall through and output 0x0E
HLT
NOP            ; padding
NOP
NOP
NOP
NOP
NOP
LDI 0xD        ; PASS code (must be at 0x0C)
OUT
HLT
NOP