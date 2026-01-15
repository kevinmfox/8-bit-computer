# Microarchitecture

[Modules](#modules)

[Instruction Cycle](#instruction-cycle)

[Fetch Cycle](#fetch-cycle)

[Control Signals](#control-signals)

[Reset & Halt](#reset--halt)

## Modules

| Module | Name                    | Inputs                 | Outputs          | Internal Storage | Description / Notes                                                              |
|--------|-------------------------|------------------------|------------------|------------------|----------------------------------------------------------------------------------|
| PC     | Program Counter         | CE, JMP, CLR           | CO               | 8-bit register   | Holds address of next instruction. Can increment or load from bus.               |
| IR     | Instruction Register    | II, CLR                | IO (Operand)     | 8-bit register   | Holds current instruction. 4 bits Opcode; 4 bits Operand. Operand may drive bus. |
| MAR    | Memory Address Register | MI, CLR                | addr -> RAM      | 8-bit register   | Holds memory address for RAM access (i.e. `RAM[MAR]`).                           |
| A      | A Register              | AI, CLR                | AO               | 8-bit register   | General-purpose register and primary ALU input.                                  |
| B      | B Register              | BI, CLR                | BO               | 8-bit register   | General-purpose register used as secondary ALU input.                            |
| ALU    | Arithmetic Logic Unit   | A, B, SU               | EO, CF, ZF       | Combinational    | Performs arithmetic operations. CF & ZF generated here but not stored.           |
| FLAGS  | Flags Register          | FI, CLR                | CF, ZF           | 2-bit register   | Latches Carry and Zero flags from ALU when `FI` is signaled.                     |
| RAM    | Random Access Memory    | RI, MAR[addr]          | RO               | SRAM             | Unified instruction/data memory addressed via `MAR`.                             |
| OUT    | Output / Display        | DI, CLR                | Display          | ATmega328P       | Latches bus value for external display.                                          |
| µSTEP  | Microstep Counter       | CLK, END, CLR          | T-State          | Counter          | Tracks current microstep (T0–T5). Reset by `END` or `CLR`.                       |
| CTRL   | Control Unit            | Opcode, Flags, T-State | Control Signals  | EEPROM + Logic   | Decodes flags, instruction (Opcode), and microstep into control signals.         |
| CLK    | Clock Module            | HLT                    | CLK              | Oscillator       | Generates system clock. Halted by `HLT` signal.                                  |

## Instruction Cycle

The CPU executes instructions using a microstep-based execution model. Each instruction is broken down into a sequence of microsteps (`T-States`) that coordinate data movement, computation, and control flow across the system.

Instruction execution proceeds as follows:
- Instructions are executed sequentially, one at a time
- Each instruction begins with a common fetch cycle (`T0` & `T1`), during which the next instruction byte is read from memory into the Instruction Register (`IR`)
- Following the fetch cycle, the instruction enters an execution phase consisting of one or more microsteps (starting at `T2`)
- The active microstep is tracked by the Microstep Counter (`µSTEP`) and identified as `T0` through `T5`
- During each microstep, the Control Unit (`CTRL`) outputs a specific set of control signals based on:
  - current Opcode (4-bits)
  - current `T-State` (3-bits via a 8-to-3 decoder)
  - current flags -- `CF`, `ZF` (2 bits)

An instruction completes when the Control Unit outputs an `END` signal. This resets the Microstep Counter, causing the next instruction cycle to begin at `T0`.

The system Clock (`CLK`) advances the microstep sequence. State-holding modules latch inputs synchronously on clock edges, while combinational logic (such as the ALU) produces outputs continuously within a cycle.

## Fetch Cycle

The fetch cycle is common to all instructions and is responsible for loading the next instruction byte from memory into the Instruction Register (`IR`). This cycle occurs at the beginning of every instruction and always starts at `T0`.

During the fetch cycle, the Program Counter (`PC`) provides the address of the next instruction, which is read from memory and latched into the Instruction Register. The Program Counter is then incremented in preparation for the following instruction.

### Fetch Cycle Microsteps

| T-State | Signals    | Description                                                                                                                                                  |
|---------|------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------|
| T0      | CO, MI     | The Program Counter places its value onto the bus (`CO`), which is loaded into the Memory Address Register (`MI`).                                           |
| T1      | RO, II, CE | The instruction byte at `RAM[MAR]` is placed onto the bus (`RO`) and loaded into the Instruction Register (`II`). The Program Counter is incremented (`CE`). |
| T2      | —          | Instruction decode phase. The Opcode is evaluated by the Control Unit to determine the execution microsteps.                                                 |

### Notes

- Opcode bits from the Instruction Register are hardwired to the Control Unit and are not placed on the bus.
- The Operand field of the Instruction Register may be used during execution, depending on the instruction.

## Control Signals

| Signal | Name                    | Direction | Module                    | Commands Using Signal               | Description                                                          |
| :----- | ----------------------- | :-------: | ------------------------- | ----------------------------------- | -------------------------------------------------------------------- |
| CO     | Counter Out             |    Out    | Program Counter (PC)      | *Fetch cycle*                       | Places the Program Counter (PC) value onto the bus.                  |
| RO     | RAM Out                 |    Out    | RAM                       | LDA, LDB, ADD, SUB, JMPV            | Places the contents of RAM[MAR] onto the bus.                        |
| IO     | Instruction Operand Out |    Out    | Instruction Register (IR) | Non-jump Operand-based instructions | Places the 4-bit Operand of the Instruction Register onto the bus.   |
| AO     | A Register Out          |    Out    | A Register                | STA, OUT, MOVB_A                    | Places the contents of register A onto the bus.                      |
| BO     | B Register Out          |    Out    | B Register                | MOVA_B                              | Places the contents of register B onto the bus.                      |
| EO     | ALU Out                 |    Out    | ALU                       | ADD, ADI, SUB                       | Places the ALU result onto the bus.                                  |
| MI     | Memory Address In       |     In    | MAR                       | LDA, LDB, STA, ADD, SUB, JMP, JMPV  | Loads the Memory Address Register (MAR) from the bus.                |
| II     | Instruction In          |     In    | Instruction Register (IR) | *Fetch cycle*                       | Loads the Instruction Register from the bus.                         |
| AI     | A Register In           |     In    | A Register                | LDA, ADD, ADI, SUB, MOVA_B, LDI     | Loads register A from the bus.                                       |
| BI     | B Register In           |     In    | B Register                | LDB, ADD, SUB                       | Loads register B from the bus.                                       |
| RI     | RAM In                  |     In    | RAM                       | STA                                 | Writes the bus value into RAM[MAR].                                  |
| DI     | Display In              |     In    | Output / Display          | OUT                                 | Latches the bus value into the output/display register.              |
| FI     | Flags In                |     In    | Flags Register            | ADD, ADI, SUB                       | Latches ALU status flags (CF, ZF).                                   |
| CE     | Counter Enable          |  Control  | Program Counter (PC)      | *Fetch cycle*                       | Increments the Program Counter by one.                               |
| JMP    | Jump                    |  Control  | Program Counter (PC)      | JMP, JMPV, JC, JZ                   | Loads the Program Counter from the bus.                              |
| END    | End Instruction         |  Control  | Control Unit              | All instructions                    | Terminates the current instruction and resets the Microstep counter. |

## Reset & Halt

### Reset (CLR)

The system supports a *Global Reset* signal that will clear and reset relevant components.

The following components are affected by Reset: Program Counter; Memory Address Register; A Register; B Register; Instruction Register; Microstep Counter

### Halt (HLT)

When a Halt Opcode is received (i.e. 0xF, 1111) a signal is sent directly to the Clock module to stop it. This effectively prevents the system from moving.