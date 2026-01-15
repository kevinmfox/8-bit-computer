## Microcode Generation

The `generate_microcode.py` Python script will generate the microcode from a given "signal truth table" (`assets\microcode.csv`) that can be put on EEPROMs.

The script can certainly be modified to take another or differntly formatted input, but the general idea is take your CPU input signals, and convert those into expected output signals.

In my case, the input signals were:
- `Opcode` - Four bit binary representation (e.g. `0101`)
- `T-Step` - Represented as `T0` through `T5` (depdnding on the instruction)
- `CF` - The state of the Carry Flag (`0` or `1`)
- `ZF` - The state of the Zero Flag (`0` or `1`)

The `T-Step`, `CF`, and `ZF` inputs can also take wildcards (`*`) which will be *expanded* to cover all posibilities.

The outputs will be whatever control signals you want active given the inputs.

For all instructions, the final step is an `END` signal, which tells the Microstep Counter to go back to `T0`.

In my case, I had my `assets\microcode.csv` file represented as active-high (i.e. 1 = ON), but the actual script will generate active-low signals, as that's what my board expected.

Note the `Address`, and `Mnemonic` fields in the `microcode.csv` file aren't actually used in the Python script, but are there simply for reference.

I programmed an Arduino to program my EEPROM chips. Steps on getting the generated microcode onto an EEPROM chip can be found [here](/arduino/readme.md#eeprom-programmer).

Sample commands:
- `python .\generate_microcode.py`

## Compiling Programs

The actual `compile.py` Python script is not much more than a generic text parser and mapping algorithm. Read a line; parse/extract/scrape certain portions; make decisions based on the inputs; output some other text. I'm not trying to downplay the importance of this, or suggest all compilers are this simple (they're not), but provided your instruction set is *simple*, and you are rigid in what you allow for your assemlby language, it's fairly straightforward. 

The compiled instructions obviously have to align with the microcode you've put on your ROMs.

So, if you give an instruction such as `ADI 0x8`, the expected compiled line would be:
- Opcode = `ADI` = `0101` or `0x5`
- Operand = `0x8` = `1000` or `0x8`
- Instruction = `0101 1000` or `0x58`

Given the limitation of a 4-bit Operand, you should be cautious when compiling programs that use `STA` instruction. This instruction will *store* the value of the A Register into the memory address of the Operand. But, if your programs runs into that same space, you may have inadvertently overwritten some of your program. My compiler doesn't check for this, and I didn't think I needed it to until I did this about a dozen times to myself ;).

The compiler supports some basic sanity checks:
- Address out of range for my architecture (i.e. referencing something above 0xF)
- Invalid instructions
- Missing Operands or Operands on instructions that expect none (e.g. `NOP`)

The compiler supports:
- Jump labels
- Using hex (e.g. `0xF`), binary (e.g. `0b1111`) or decimal (e.g. `15`) for Operands
- Comments

Given a text file input (by convention I've labelled these ASM files), it will print the compiled code to screen. This can be copied/pasted into the `programs.h` file for the [Arduino Programmer](/arduino/readme.md#ram-programmer).

I used an Arduino to program my RAM chip while it was on the board. Steps on getting these programs onto the board can be found [here](/arduino/readme.md#ram-programmer).

Sample commands:
- `python .\compile.py --program-name TEST assets\program-test01.asm`
- `python .\compile.py --program-name DEMO01 --print-summary .\assets\program-demo-01.asm`