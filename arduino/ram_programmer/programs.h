#pragma once
#include <Arduino.h>
#include <avr/pgmspace.h>

struct ProgramDef {
  const char* name;          // string literal (OK for small menus)
  const uint8_t* bytes;      // points to PROGMEM data
  uint8_t len;               // derived from sizeof(program)
};

// --------------------------------
// PROGRAM TABLE/LOOKUP
// --------------------------------
// This table needs to be updated as per the actual programs below
#define PROGRAM_LIST(X) \
  X(TEST01,     "Flags & Jumps") \
  X(TEST02,     "Memory") \
  X(TEST03,     "Registers & ADD") \
  X(TEST04,     "JMPV") \
  X(DANCE01,    "LED Dance 1") \
  X(DANCE02,    "LED Dance 2") \
  X(DEMO01,     "Demo 01")

// --------------------------------
// PROGRAM LIST
// --------------------------------

const uint8_t TEST01[16]PROGMEM = {
  // ; Flags & Jumps test
  // ; Will test: LDI, STA, SUB, ADI, JC, JZ, JMP, OUT, HLT
  // ; PASS output: 0x0A
  // ; FAIL output: 0x0E
  0xD1, // LDI 0x01 ; A = 1
  0x3F, // STA 0x0F ; RAM[0xF] = 1
  0xD0, // LDI 0x00 ; A = 0
  0xEF, // SUB 0x0F ; A = 0 - 1 = 0xFF, CF=0, ZF=0
  0x8D, // JC 0x0D ; must NOT jump (CF=0)
  0x51, // ADI 0x01 ; A = 0x00, CF=1, ZF=1
  0x98, // JZ 0x08 ; must jump (ZF=1)
  0x6D, // JMP 0x0D ; FAIL if we get here
  0x8A, // JC 0x0A ; must jump (CF=1)
  0x6D, // JMP 0x0D ; FAIL if we get here
  0xDA, // LDI 0x0A ; PASS code
  0xA0, // OUT
  0xF0, // HLT
  0xDE, // LDI 0x0E ; FAIL code
  0xA0, // OUT
  0xF0, // HLT
};

const uint8_t TEST02[12]PROGMEM = {
  // ; Memory test
  // ; Will test: LDI, STA, LDA, LDB, MOVA_B, OUT, HLT
  // ; PASS output: 0x0B
  // ; FAIL output: not 0x0B
  0xD9, // LDI 0x09 ; A = 0x09 (signature value)
  0xA0, // OUT ; just to set it to an incorrect value
  0x3E, // STA 0x0E ; RAM[0x0E] = 0x09
  0xD3, // LDI 0x03 ; A = 0x03 (signature value)
  0x3F, // STA 0x0F ; RAM[0x0F] = 0x03
  0x1E, // LDA 0x0E ; A = 0x09 (proves STA -> LDA round-trip)
  0x52, // ADI 0x02 ; A = 0x0B
  0x2F, // LDB 0x0F ; B = 0x03 (loads B from RAM)
  0xB0, // MOVA_B ; A = 0x03 (proves B truly received 0x03)
  0x58, // ADI 0x08 ; A = 0x0B
  0xA0, // OUT ; expect 0x0B
  0xF0, // HLT
};

const uint8_t TEST03[10]PROGMEM = {
  // ; Register transfer & ADD test
  // ; Will test: NOP, LDI, STA, ADD, MOVB_A, MOVA_B, OUT, HLT
  // ; PASS output: 0x0C
  0x00, // NOP ; do nothing
  0xD1, // LDI 0x01 ; A = 1
  0x3F, // STA 0x0F ; RAM[0x0F] = 1
  0xDB, // LDI 0x0B ; A = 0x0B
  0x4F, // ADD 0x0F ; A = 0x0C        (tests ADD-from-RAM)
  0xC0, // MOVB_A ; B = 0x0C        (tests MOVB_A)
  0xD0, // LDI 0x00 ; A = 0
  0xB0, // MOVA_B ; A = 0x0C        (tests MOVA_B)
  0xA0, // OUT ; expect 0x0C
  0xF0, // HLT
};

const uint8_t TEST04[16]PROGMEM = {
  // ; JMPV test
  // ; Will test: LDI, STA, JMPV, OUT, HLT
  // ; PASS output: 0x0D
  0xDC, // LDI 0x0C ; A = 0x0C (address of PASS block)
  0x3F, // STA 0x0F ; RAM[0x0F] = 0x0C (jump vector)
  0xDE, // LDI 0x0E ; preload FAIL code in A
  0x7F, // JMPV 0x0F ; PC = RAM[0x0F] (should jump to 0x0C)
  0xA0, // OUT ; if JMPV fails, we fall through and output 0x0E
  0xF0, // HLT
  0x00, // NOP ; padding
  0x00, // NOP
  0x00, // NOP
  0x00, // NOP
  0x00, // NOP
  0x00, // NOP
  0xDD, // LDI 0x0D ; PASS code (must be at 0x0C)
  0xA0, // OUT
  0xF0, // HLT
  0x00, // NOP
};

const uint8_t DANCE01[14]PROGMEM = {
  // ; Dual-chase display loop
  // ; Visuals:
  // ; - A counts upward
  // ; - B shows (0xFF - A), so it "counts downward"
  // ; - RAM[0] mirrors A, RAM[1] mirrors B (RAM+MAR animate)
  // ; - ALU active every step
  0xD1, // LDI 0x01
  0x3F, // STA 0x0F ; RAM[0xF] = 1  (increment constant)
  0xD0, // LDI 0x00
  0x30, // STA 0x00 ; RAM[0] = A          (show A on RAM)
  0xC0, // MOVB_A ; B = A               (copy A so B changes too)
  0xD0, // LDI 0x00
  0xE0, // SUB 0x00 ; A = 0 - RAM[0] = (-A)   (two's complement-ish step)
  0xEF, // SUB 0x0F ; A = A - 1 = (~A)        (because (-A) - 1 == ~A)
  0xC0, // MOVB_A ; B = A               (B = ~original A)
  0x31, // STA 0x01 ; RAM[1] = B          (show B on RAM)
  0x10, // LDA 0x00 ; A = original A back
  0x4F, // ADD 0x0F ; A = A + 1           (increment)
  0xA0, // OUT
  0x63, // JMP 0x03
};

const uint8_t DANCE02[10]PROGMEM = {
  // ; ALU shimmer loop
  // ; Visuals:
  // ; - A runs and jitters
  // ; - B periodically captures A (so B LEDs “follow” but lag)
  // ; - RAM[1] stores a constant (used for ADD/SUB)
  // ; - ALU is constantly active
  0xD7, // LDI 0x07
  0x31, // STA 0x01 ; RAM[1] = 7  (constant)
  0x51, // ADI 0x01 ; A++
  0xA0, // OUT
  0xC0, // MOVB_A ; B = A       (B tracks A)
  0x41, // ADD 0x01 ; A = A + 7   (big jump, ALU lights)
  0xE1, // SUB 0x01 ; A = A - 7   (back, but flags toggle)
  0x52, // ADI 0x02 ; A += 2
  0xA0, // OUT
  0x62, // JMP 0x02
};

const uint8_t DEMO01[16]PROGMEM = {
  // ; Fibonacci sequence
  // ; Setup
  0xD1, // LDI 0x01 ; Set A = 1 initially
  0x3E, // STA 0x0E ; Store A = 1 into 0xE (Previous value)
  0x3F, // STA 0x0F ; Store A = 1 into 0xF (Current value)
  // ; Main loop
  0x8C, // JC 0x0C ; If Carry Flag is set, be done
  0x1F, // LDA 0x0F ; Load current value into A
  0x3D, // STA 0x0D ; Put that into temp storage
  0x4E, // ADD 0x0E ; Add our previous value
  0xA0, // OUT ; Show it on the Display
  0x3F, // STA 0x0F ; Store our current value
  0x1D, // LDA 0x0D ; Load our temporary value
  0x3E, // STA 0x0E ; Store that into our previous value
  0x63, // JMP 0x03 ; Keep on going
  0xF0, // HLT
  0x00, // PAD
  0x00, // PAD
  0x00, // PAD
};

// --- Auto-generate PROGRAMS[] from PROGRAM_LIST ---
#define MAKE_PROG(sym, title) { title, sym, (uint8_t)sizeof(sym) },
const ProgramDef PROGRAMS[] = {
  PROGRAM_LIST(MAKE_PROG)
};
#undef MAKE_PROG

const uint8_t PROGRAM_COUNT = sizeof(PROGRAMS) / sizeof(PROGRAMS[0]);