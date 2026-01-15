# The Build

<img src="/images/main02.jpg" width="600">

## Inventory

Here's a rundown of all the parts I used. It should be *pretty close* to accurate ;)

Notes:
- I only used a single 100 nF decoupling capacitor per breadboard module. I know *best practice* is to have a decoupling capacitor on every IC. I didn't have any issues, but take note.
- I used [BB830 breadboards](https://www.amazon.ca/BusBoard-Prototype-Systems-BB830-Solderless/dp/B0040Z4QN8) almost exclusively (I had a couple laying around before I started). Seriously, if you're going to do this, don't go cheap on breadboards. I tried saving a couple dollars at a local electronics shop, and the boards were inconsistent at best.
- For wiring I bought two of [these kits](https://www.amazon.ca/dp/B0CM2Y7V1Z). For those interested, I used: 10.4 feet or red wire, 13.7 feet of white wire, 14.3 feet of black wire, 30.0 feet of blue wire, 57.3 feet of yellow wire, and 59.0 feet of green wire. 184.7 feet of wire (not account for mistakes or rework!)

| Type          | Value           | Quantity | Notes |
|---------------|-----------------|----------|-------|
| Capacitor     | 1 µF            | 1        | 555 astable capacitor |
| Capacitor     | 10 nF           | 3        | 555 signal conditioning |
| Capacitor     | 100 nF          | 14       | Decoupling capacitors - one per breadboard module |
| Capacitor     | 22 pF           | 2        | For the 16 MHz Crystal on the ATmega driving the Display |
| LED           | 5 mm Yellow     | 6        | Generally used for Flags and a few Control indicators |
| LED           | 5 mm Green      | 21       | Program Counter, Operand, and input Control indicators |
| LED           | 5 mm Red        | 42       | MAR, A & B Registers, ALU result, and Bus indicators |
| LED           | 5 mm White      | 8        | Only used on the RAM module |
| LED           | 5 mm Blue       | 17       | Clock pulse, Opcode, Microstep Counter, and output Control indicators |
| Resistor      | 220 Ω           | 95       | LED current-limiting resistors, including the 1602A display |
| Resistor      | 1 kΩ            | 3        | 555 astable timing (R1), 555 monostable debounce, and 1602A display contrast |
| Resistor      | 330 Ω           | 1        | 555 astable timing (R2) |
| Resistor      | 1 MΩ            | 1        | 555 monostable timing |
| Resistor      | 4.7 kΩ          | 6        | Various pull-ups and pull-downs |
| Potentiometer | 0-200 kΩ        | 1        | Clock speed adjustment (555 astable) |
| Switch        | Slide switch    | 2        | Clock mode, and Control Unit enable/disable |
| Push Button   | Momentary       | 3        | Clock step, Display mode, and Reset buttons |
| Display       | 1602A LCD       | 1        | Arduino-driven output display |
| Crystal       | 16 MHz          | 1        | ATmega328P clock source |
| IC            | LM555           | 3        | Clock module (astable, monostable, selector) |
| IC            | ATmega328P      | 1        | Display controller |
| IC            | 74LS08          | 3        | AND gates - Clock, ALU, and Control Unit logic |
| IC            | 74LS245         | 6        | 8-bit bus transceivers |
| IC            | 74LS161         | 3        | Binary Counter - Program Counter, Microstep Counter |
| IC            | 74LS173         | 10       | Register data - MAR, A, B, IR, Flags |
| IC            | 74HC14          | 2        | Schmitt trigger inverters |
| IC            | 74LS32          | 1        | OR gates -  Clock |
| IC            | 74LS02          | 2        | NOR gates - RAM signal logic, ALU |
| IC            | 74LS138         | 2        | 3-to-8 Decoder - IR, Microstep Counter |
| IC            | 74LS283         | 2        | Binary adders - ALU |
| IC            | 74LS86          | 2        | XOR gates - ALU subtraction |
| Memory        | AS6C6264 SRAM   | 1        | Main RAM |
| Memory        | AT28C64B EEPROM | 2        | Microcode ROMs |
| Breadboard    | BB830           | 14       | To connect everything |

## Modules

[Clock Module](#clock-module)

[Program Counter](#program-counter)

[Display](#display)

[Memory Address Register (MAR)](#memory-address-register-mar)

[Random Access Memory (RAM)](#random-access-memory-ram)

[A & B Registers](#a--b-registers)

[Arithmetic Logic Unit (ALU) & Flags](#arithmetic-logic-unit-alu--flags)

[Instruction Register](#instruction-register)

[Microstep Counter](#microstep-counter)

[Auxiliary Board](#auxiliary-board)

[Control Unit](#control-unit)

## Clock Module

The Clock module was an interesting aspect of this build because it serves a distinctly unique purpose compared to all other modules. It's sole purpose is to *emit* a clock pulse - that's it. It could have been a single 555 timer chip, a capacitor, and a couple resistors. It's the ability to select between modes (astable vs. mono), adjust the speed via the potentiometer, and halt the clock that accounts for the bulk of the complexity.

With the exceptions that I kept the LEDs to indicate the astable tick and the mono step, and I used 74HC14 Schmitt invertger to *clean up* and invert my `HLT` and `CLK` signals, what I built was right out of [Ben Eater's tutorials](https://eater.net/8bit/clock).

<img src="/images/clock02.jpg" width="600">

- #1 - Potentiometer to adjust the clock speed
- #2 - Pushbutton to send a single pulse (assuming in mono mode)
- #3 - Astable LED
- #4 - Monostable LED
- #5 - Slider to control astable or mono mode
- #6 - Final clock output

<img src="/images/clock_sch01.jpg" width="600">

## Program Counter

Not much difference here vs. Ben's build with the exception that I made my Program Counter a full 8 bits vs. Ben's 4 bits. Practically speaking, this made little difference, but it did give me the option to do a full 8-bit IR later on (which I didn't), and also allowed me to run the program space beyond 16 bits (which I did on occasion).

I messed this portion of the build up twice! This was the second module I worked on, so I hadn't fully conceptualized the build in my head yet (excuses). My bus runs MSB (left) to LSB (right). The 74LS245 chip, when placed PIN 1 bottom-left, runs opposite - LSB > MSB  (if you're using PIN numbers and labels as your guide). Additionally, the 74LS161s (assuming PIN 1 bottom-left) also runs the same as the 74LS245 - LSB > MSB. I was testing my `JMP` command, and realized the data coming back in from the bus was backwards. So, I ripped it all up, rewired it, and then the data coming from the bus was correct, but the data coming from the Program Counter was entering the bus backwards *sigh*. Got it right on the third attempt.

<img src="/images/pc01.jpg" width="600">

<img src="/images/pc_sch01.jpg" width="600">

## Display

A somewhat controversial *pivot* here ;). This deviated dramatically from Ben's initial build. I *cheated* and used an ATmega328P to run my display output. This (clearly) wasn't about saving space, wires, or chips, but more that I wanted to use the 1602A LED display I had sitting around, and also wanted to be able to view hex, decimal, and binary in a single spot - this solved that. 

I incorporated a pushbutton that would allow me to switch between modes. And I also programmed PIN 19 on the ATmega to allow me to constantly *pull* the bus if it was enabled (wired to ground or pulled low).

<img src="/images/display02.jpg" width="600">

- #1 - The notorious ATmega chip
- #2 - Pushbutton to change display modes
- #3 - PIN 19 on the ATmega to allow pulling from the bus directly (shown in the image below)

<img src="/images/display03.jpg" width="600">

<img src="/images/display_sch01.jpg" width="600">

## Memory Address Register (MAR)

Because I had a full 8-bit Program Counter, I needed an 8-bit Memory Address Register (what the Program Counter puts onto the bus the MAR picks up). I also opted not to have DIP switches on my build for either the MAR or the RAM. This made initial testing a little more difficult, but it did give me more space to work with.

I also had my status LEDs on this board. This made sense as a) there was actually some room here; and, b) it was close to the clock which received the `HLT` signal.

<img src="/images/mar02.jpg" width="600">

- #1 - Status LEDs. Green = running; Red = `/HLT` received (i.e. stop the clock output)

<img src="/images/mar_sch01.jpg" width="600">

## Random Access Memory (RAM)

Ben's build *called for* some 74189 chips for the RAM. I couldn't find those readily available, so I went with a single AS6C264 chip. This was also one of the few chips I *rotated* (PIN 1 top-right) so that it was easier to run my wiring (MAR above).

I used white LEDs for the RAM, and although this seemed *neat* at the time, I'll probably swap them for anything but. I've only got 220 resistors on them, and they can be blinding if I'm standing over them.

Although not stricly required, I gated my RAM with a 74LS245 chip. I needed this as I wanted the LEDs to always show what the RAM was seeing. This required the `/OE` PIN on the RAM to be enabled by default. Without the 74LS245, it would always be driving the bus.

Finally, you'll notice the *rats nest* of wiring around the 74LS02 NOR chip. I was quite happy that I was able to send two signals (`/RI`, `/RO`) up to this chip, and it would do the logic to control the AS6C264 RAM signals (`/OE`, `/WE`) and the 74LS245 signals (`DIR`, `/CE`).

<img src="/images/ram02.jpg" width="600">

- #1 - 74LS02 NOR gate used to perform *magic*. Logic can be seen in the image below.

<img src="/images/ram_sch01.jpg" width="600">

## A & B Registers

I think these two registers were piece-for-piece identical to Ben Eater's build. Same chips, same LEDs, same signals. 

If you're going to take on this project (or similar), I recommend *batch building* your registeres.

<img src="/images/a_register01.jpg" width="600">

<img src="/images/b_register01.jpg" width="600">

<img src="/images/a_register_sch01.jpg" width="600">

## Arithmetic Logic Unit (ALU) & Flags

I'm happy to say that my ALU almost worked the first time I hooked it up...almost. After spending a few hours double checking to ensure the ALU was getting the *right information* from my registers (it wasn't), I started probing PINs on the 74LS283s and 74LS86s until I could find what was wrong. Like most *breadboard stories*, it turned out that I had a single wire in *off by one* column. As you can imagine, in the world of binary math, that's enough to cause chaos.

This is probably my *messiest* board, and if I had the patience, I'd go back and rewire some of it. I also considered trying to fit the entire ALU on the right side of the build, but after some scenario planning, this was the location that made the most sense for me.

The Subtract flag (`SU`) is indicated on the right-side of the build. The Carry and Zero flags (`CF`, `ZF`) are indicated on the left side of the build.

<img src="/images/alu_right02.jpg" width="600">

- #1 - The Subtract flag indicator

<img src="/images/alu_left02.jpg" width="600">

- #1 - The Carry & Zero flag indicators

<img src="/images/alu_full02.jpg" width="600">

<img src="/images/alu_sch01.jpg" width="600">

<img src="/images/flags_sch01.jpg" width="600">

## Instruction Register

The Instruction Register itself is largely the same as what Ben Eater had for his build. Where it gets a little fuzzier is how I handled a couple additional Opcodes in hardware instead of at the Control Unit.

I really wanted to add the B Register Output (`/BO`) signal, and I also wanted to have an `/END` signal to tell my Microstep Counter to reset (instead of relying on it wrapping around). But, I only had 16 signals available to me, so I needed to either *drop* two signals, or find a way to address those outside the Control Unit. That's where the 74LS138 3-to-8 decoder comes in. I send it my 4-bit Opcode, and I also *tie* `/T2` from the Microstep Counter to one of it's enable PINs (`E1`). So, the chip only *comes alive* during `/T2` (which is where instructions are interpreted). If it receives an Opcode of 1111 (`/HLT`) it outputs LOW on PIN 7, and if it receives an Opcode of 1110 (`/SU`) it outputs LOW on PIN 6. For `/HLT`, I don't need to do anything special except send that up to the Clock module - everything stops. For `/SU`, that goes into an inverter and then gets latched on a seperate 74LS173 that is also enabled on `/T2`.

The other piece worth noting (mostly because I forgot until I was troubleshooting), is that the MSB nibble for the Instruction Register needs to be tied low or grounded (i.e. it can't send data to the bus). I suppose this only matters if you're allowing jump (`JMP`) instructions (which I am). The problem is that the Instruction receives data such as `01101010`, the MSB nibble is `0110`, which is the jump instruction. The LSB nibble is `1010` which is the address to jump to, and needs to be loaded into the Program Counter. If you output the entire Instruction Register to the bus (i.e. `01101010`), you'll actually be jumping to that address instead of `00001010` (which is what you want). This is obvious, but when my *task for the night* was to wire everything to the bus, I did just that :).

<img src="/images/ir02.jpg" width="600">

- #1 - MSB nibble *tied low* so the Instruction Register can only output the LSB nibble

<img src="/images/ir03.jpg" width="600">

- #1 - The 74LS138 I used to decode `/SU` and `/HLT`

<img src="/images/ir_sch01.jpg" width="600">

## Microstep Counter

Ignoring the *extra* 74LS138 (discussed above), this is pretty close to what Ben Eater had for his Microstep Counter. The only real modification I made was that I have a `/CLR_END` signal going into PIN 1 of the 74LS161. This will trigger if the Reset switch is pressed, or an `/END` control is signaled from the Control Unit (which happens at the end of every program step). Oh, and I flipped my LEDs to be active-low so only the current TStep was *on*.

<img src="/images/microstep01.jpg" width="600">

<img src="/images/microstep_sch01.jpg" width="600">

## Auxiliary Board

For the most part, this board responsible for cleaning up or combining signals before they went *further up the board*.

Note that there's no dedicated schmatic for this board, as the chips are accounted for in other areas.

The 74LS08 (AND Gate) was only used to combine the `/END` (which is HIGH when it's off) and the `/CLR` (also HIGH when it's off) signals. If either of these went LOW, the resulting `CLR_END` signal would be LOW causing the Microstep Counter to reset. 

The 74HC14 (Schmitt Inverter) was used to invert the initial `/CLR` signal to produce a *clean* `CLR` signal (used by most of the board), and then again to produce a *clean* `/CLR` signal to be used by the Program Counter. It also inverted `/CE` (for the Program Counter) to `CE` (as it expects active-high).

The 74LS173 was only used to latch the value of `SU` (Subtract), and instead of latching on a Clock pulse, it latched on `/T2` from the Microstep Counter.

<img src="/images/auxiliary01.jpg" width="600">

## Control Unit

The big one! My input logic and wiring was, I think, the same as Ben's here. Three wires from the Microstep Counter, four wires to represent the Opcode (from the Instruction Register), and two wires for the Flags (`CF`, `ZF`) - 9 inputs total.

I also utilized two separate EEPROM chips to take those 9 inputs and turn them into 8 + 8 = 16 total outputs.

I differed in the following areas...

I used two AT28C64B chips instead of 28C16s - I just couldn't find them easily.

My `/HLT` and `/SU` signals were taken care of *in hardware* directly instead of on the controller (this is addressed in #instruction-register).

I programmed all my control signals as active-low instead of active-high. This was a trade-off between more hardware to inverte high signals (as most of the chips wanted low signals), or working with potentially unintuitive programming where everything is a 1 except when it needs to do something - then it's a 0. Even in the truth table that I used to program the EEPROMs, I represent *active* as HIGH/1. Only when my `generate_microcode.py` Python script takes that truth table input and inverts it all is everything represetned as active-low proper.

For actual programming, I'd remove the chips from the board, put them onto [my programmer](/arduino/readme.md#eeprom-programmer), program them one at a time, then put them back onto the board. If I had more space, and had planned ahead, I might have put a *chip select* switch somewhere on the board to allow for easier programming.

The slide switch (#1 below) simply enables or disables the Control Unit via PIN 20 (`/CE`). I introduced this after continuously removing them to test my RAM. The problem is that, once power is supplied to the board (which you kind of need for RAM programming), the Control Unit is almost always trying to do something, and most (all) of those things get in the way of writing to or reading from RAM. So, my [RAM programmer](/arduino/readme.md#ram-programmer) would fail.

<img src="/images/control02.jpg" width="600">

I'm happy with the way this LED panel turned out. I ordered mine by *Outputs*, *Inputs*, and *Other* control signals. And, of course, I color coded them :)

<img src="/images/control_leds02.jpg" width="600">

#1 - This seemed like the right spot for this Reset switch

<img src="/images/control_and_leds01.jpg" width="600">

<img src="/images/control_sch01.jpg" width="600">

## Assembly

For the most part, I assembled and built the project in the order described above. There are some progress photos [here](progress-photos.md).

At a high level, my build went as follows:
- Build and test the Clock.
- Build and test the Program Counter running of the Clock.
- Build the A & B Registers. Manually test their ability to latch input, clear, and put output onto the bus.
- Build the ALU, and manually test it's ability to take data from A & B Registers, and put output on to the bus. Worry about how you'll display it's data until later ;).
- Build and test the Display.
- Build and test the MAR.
- Build the RAM, connect it to the MAR, and test the interactions.
- Build the Instruction Register and test it's abilities (clear, take input, push to bus). Also, try not to forget to tie the MSB low going back to the bus (see [notes here] (#instruction-register));).
- Build the Microstep Counter, and make sure it can count. Also remember to have it active on `/CLK` vs. `CLK` everywhere else.
- Wire everything to the bus.
- Build and layout the the Control Unit
- Run control wires everywhere, and test every single one of them!
- [Write and test a program](/python/readme.md#microcode-generation) to convert your Control Unit truth table to actual EEPROM-usuable code.
- [Program](/arduino/readme.md#eeprom-programmer) the EEPROMs.
- Manually test some Opcodes to ensure signals are working as expected as you step through an instruction.
- [Write a program](/python/readme.md#compiling-programs) to generate *machine code* from your given instruction set/assembly language.
- [Program](/arduino/readme.md#ram-programmer) the RAM.
- Troubleshoot little things for days ;)