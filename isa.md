# Instruction Set Architecture (ISA)

Below is the instruction set architecture (ISA) for this CPU build.

It's a *mostly* **8-bit system**, with an **8-bit Memory Address Register**, but instruction Operands are limited to 4 bits.

All components share a single **8-bit data bus**.

It's entirely deterministic and sequential in nature.

[Memory Model](#memory-model)

[Registers](#registers)

[Flags](#flags)

[Execution Cycle](#execution-cycle)

[Instruction Set - Summary](#instruction-set---summary)

[Instruction Set - Details](#instruction-set--details)
- [Fetch Cycle](#fetch-cycle)
- [NOP](#nop)
- [LDA](#lda)
- [LDB](#ldb)
- [STA](#sta)
- [ADD](#add)
- [ADI](#adi)
- [JMP](#jmp)
- [JMPV](#jmpv)
- [JC](#jc)
- [JZ](#jz)
- [OUT](#out)
- [MOVA_B](#mova_b)
- [MOVB_A](#movb_a)
- [LDI](#ldi)
- [SUB](#sub)
- [HLT](#hlt)

[Undefined Behaviour](#undefined-behaviour)

## Memory Model

The CPU provides a unified memory model for instructions and data.

- Memory is byte-addressable
- Each memory location holds an 8-bit value
- Instructions and data share the same address space

### Addressing

- Instruction Operands that reference memory (`addr`, `vec`) are **4 bits**
- As a result, instructions can directly reference addresses in the range `0x0`–`0xF`
- The Memory Address Register (MAR) is 8 bits wide; however, only the lower 4 bits are architecturally defined for instruction-driven memory access

### Addressing Notes

- Instructions that use `addr` or `vec` Operands only access memory locations `0x0`–`0xF`
- Behavior for memory accesses outside this range is undefined
- The CPU does not distinguish between instruction memory and data memory

## Registers

| Name | Type             | Description                                                                                                               |
| :--- | ---------------- | ------------------------------------------------------------------------------------------------------------------------- |
| A    | 8-bit data       | Accumulator. Primary ALU Operand and destination. Most arithmetic and logic results are stored here.                      |
| B    | 8-bit data       | Secondary ALU Operand. Typically loaded from memory before arithmetic operations.                                         |
| PC   | 8-bit special    | Program Counter. Holds the address of the next instruction.                                                               |
| IR   | 8-bit internal   | Instruction Register. Holds the current instruction byte -- 4-bit Opcode + 4-bit Operand -- during decode and execution.  |
| MAR  | 8-bit internal   | Memory Address Register. Holds the address for the current memory access.                                                 |

## Flags

| Flag | Name       | Description                                                                                                                 |
| :--- | ---------- | --------------------------------------------------------------------------------------------------------------------------- |
| ZF   | Zero Flag  | Set (`1`) when the result of an ALU operation is `0x00`. Cleared (`0`) otherwise.                                           |
| CF   | Carry Flag | Set (`1`) when an arithmetic operation produces a carry out (addition) or no borrow (subtraction). Cleared (`0`) otherwise. |

**Notes:**
- Flags are updated only by ALU operations (ADD, ADI, SUB).
- CF reflects the carry-out of the most significant bit for addition, and the inverse of borrow for subtraction.
- Flags are evaluated by conditional jump instructions (JC, JZ).

## Execution Cycle

The CPU relies on a combination of Flags (CF & ZF), the current Opcode (e.g. 0101), and the Microstep Counter position (T0-T5) to determine the signals to be sent.

For each instruction the following events occur:
- T0: The value of the Program Counter (PC) is loaded into the Memory Address Register (MAR)
- T1: The contents of RAM at the position referenced by the MAR is put on the bus and loaded into the Instruction Register (IR). The PC is incremented by 1.
- T2-T5: The number of steps vary depending on the instruction, but the final step will always include an END signal to reset the Microstep Counter to T0.

The only exception to the above sequence is the HLT (1111) command. It does have an END signal on T3 for consistency, but T2 will stop the Clock completely.

## Instruction Set - Summary

Instructions are loaded into the 8-bit **Instruction Register (IR)**.

The first 4 bits (MSBs) are the Opcode; the second 4 bits (LSBs) are the Operand.

If an instruction does not require an Operand, the Operand bits are ignored for that instruction.

Operand descriptions:
- addr: 4-bit address
- imm: 4-bit immediate value that will be used directly
- vec: 4-bit address for an immediate jump (practically the same as addr)

Conventions:
- RAM[addr]: means take the value of RAM at addr
- A ← B: means put the vaulue of B into A

| Opcode       | Mnemonic | Operand | Description         |
| ------------ | -------- | ------  | ------------------- |
| `0x0 (0000)` | NOP      | —       | No operation        |
| `0x1 (0001)` | LDA      | addr    | A ← RAM[addr]       |
| `0x2 (0010)` | LDB      | addr    | B ← RAM[addr]       |
| `0x3 (0011)` | STA      | addr    | RAM[addr] ← A       |
| `0x4 (0100)` | ADD      | addr    | A ← A + RAM[addr]   |
| `0x5 (0101)` | ADI      | imm     | A ← A + imm         |
| `0x6 (0110)` | JMP      | addr    | PC ← addr           |
| `0x7 (0111)` | JMPV     | vec     | PC ← RAM[vec]       |
| `0x8 (1000)` | JC       | addr    | PC ← addr if CF = 1 |
| `0x9 (1001)` | JZ       | addr    | PC ← addr if ZF = 1 |
| `0xA (1010)` | OUT      | —       | Output A            |
| `0xB (1011)` | MOVA_B   | —       | A ← B               |
| `0xC (1100)` | MOVB_A   | —       | B ← A               |
| `0xD (1101)` | LDI      | imm     | A ← imm             |
| `0xE (1110)` | SUB      | addr    | A ← A − RAM[addr]   |
| `0xF (1111)` | HLT      | —       | Halt CPU            |

## Instruction Set – Details

---

### Fetch Cycle

| T  | CO | RO | MI | II | CE |
|----|----|----|----|----|----|
| T0 | 1  |    | 1  |    |    |
| T1 |    | 1  |    | 1  | 1  |

- Applies to all instructions (`T0` and `T1`)
- Instruction execution begins at `T2`

---

### NOP

Description: No operation  
Code: `0x0`; `0000`

| T  | END |
|----|-----|
| T2 | 1 |

- Performs no state changes

---

### LDA

Description: Load A from `RAM[MAR]`
Code: `0x1`; `0001`

| T  | IO | MI | RO | AI | END |
|----|----|----|----|----|-----|
| T2 | 1  | 1  |    |    |     |
| T3 |    |    | 1  | 1  |     |
| T4 |    |    |    |    | 1   |

---

### LDB

Description: Load B from `RAM[addr]`
Code: `0x2`; `0010`

| T  | IO | MI | RO | BI | END |
|----|----|----|----|----|-----|
| T2 | 1  | 1  |    |    |     |
| T3 |    |    | 1  | 1  |     |
| T4 |    |    |    |    | 1   |

---

### STA

Description: Store A into `RAM[addr]`
Code: `0x3`; `0011`

| T  | IO | MI | AO | RI | END |
|----|----|----|----|----|-----|
| T2 | 1  | 1  |    |    |     |
| T3 |    |    | 1  | 1  |     |
| T4 |    |    |    |    | 1   |

---

### ADD

Description: Add `RAM[addr]` value to A  
Code: `0x4`; `0100`

| T  | IO | MI | RO | BI | EO | AI | FI | END |
|----|----|----|----|----|----|----|----|-----|
| T2 | 1  | 1  |    |    |    |    |    |     |
| T3 |    |    | 1  | 1  |    |    |    |     |
| T4 |    |    |    |    | 1  | 1  | 1  |     |
| T5 |    |    |    |    |    |    |    | 1   |

- Updates `CF` and `ZF` based on ALU result

---

### ADI

Description: Add immediate Operand to A  
Code: `0x5`; `0101`

| T  | IO | BI | EO | AI | FI | END |
|----|----|----|----|----|----|-----|
| T2 | 1  | 1  |    |    |    |     |
| T3 |    |    | 1  | 1  | 1  |     |
| T4 |    |    |    |    |    | 1   |

- Immediate value comes from instruction Operand
- Updates `CF` and `ZF` based on ALU result

---

### JMP

Description: Jump to Operand address
Code: `0x6`; `0110`

| T  | IO | JMP | END |
|----|----|-----|-----|
| T2 | 1  | 1   |     |
| T3 |    |     | 1   |

---

### JMPV

Description: Jump via vector (indirect)
Code: `0x7`; `0111`

| T  | IO | MI | RO | JMP | END |
|----|----|----|----|-----|-----|
| T2 | 1  | 1  |    |     |     |
| T3 |    |    | 1  | 1   |     |
| T4 |    |    |    |     | 1   |

- Jump target is read from `RAM[vec]`
- The vector address is limited to 4 bits (≤ `0x0F`) due to Instruction Register Operand width

---

### JC

Description: Jump if Carry Flag (`CF`) set  
Code: `0x8`; `1000`

| T  | CF | IO | JMP | END |
|----|----|----|-----|-----|
| T2 | 1  | 1  | 1   |     |
| T3 | 1  |    |     | 1   |

- If `CF` is not set, `T2` will signal `END`

---

### JZ

Description: Jump if Zero Flag (`ZF`) set  
Code: `0x9`; `1001`

| T  | ZF | IO | JMP | END |
|----|----|----|-----|-----|
| T2 | 1  | 1  | 1   |     |
| T3 | 1  |    |     | 1   |

- If `ZF` is not set, `T2` will signal `END`

---

### OUT

Description: Output from A
Code: `0xA`; `1010`

| T  | AO | DI | END |
|----|----|----|-----|
| T2 | 1  | 1  |     |
| T3 |    |    | 1   |

- Latches A into the output/display register

---

### MOVA_B

Description: Move B into A  
Code: `0xB`; `1011`

| T  | BO | AI | END |
|----|----|----|-----|
| T2 | 1  | 1  |     |
| T3 |    |    | 1   |

---

### MOVB_A

Description: Move A into B  
Code: `0xC`; `1100`

| T  | AO | BI | END |
|----|----|----|-----|
| T2 | 1  | 1  |     |
| T3 |    |    | 1   |

---

### LDI

Description: Load immediate Operand into A  
Code: `0xD`; `1101`

| T  | IO | AI | END |
|----|----|----|-----|
| T2 | 1  | 1  |     |
| T3 |    |    | 1   |

- Immediate value comes from instruction Operand

---

### SUB

Description: Subtract `RAM[addr]` value from A  
Code: `0xE`; `1110`

| T  | IO | MI | RO | BI | EO | AI | FI | END |
|----|----|----|----|----|----|----|----|-----|
| T2 | 1  | 1  |    |    |    |    |    |     |
| T3 |    |    | 1  | 1  |    |    |    |     |
| T4 |    |    |    |    | 1  | 1  | 1  |     |
| T5 |    |    |    |    |    |    |    | 1   |

- Updates `CF` and `ZF` based on ALU result

---

### HLT

Description: Halt CPU
Code: `0xF`; `1111`

| T  | HLT | END |
|----|-----|-----|
| T2 | 1   |     |
| T3 |     | 1   |

- Stops instruction execution until a Reset or Power Cycle is performed
- `T3` should never be reached, and only signals `END` for instruction consistency

## Undefined Behaviour

The following behaviors are undefined by the ISA. 

Programs relying on any of these behaviors are not portable and may not execute consistently across implementations.

- Execution of Opcodes not defined in this specification
- Reliance on the value of ignored Operand bits for instructions that do not use an Operand
- Reliance on the state of flags after instructions that do not explicitly update them
- Execution behavior after a `HLT` instruction, other than the requirement that instruction execution stops