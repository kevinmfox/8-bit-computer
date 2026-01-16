<img src="/images/main01.jpg" width="600">

<img src="/images/architecture02.jpg" width="600">

Credit to [diagrams.net](https://app.diagrams.net) for making the above image easy to do.

## Background

I came across Ben Eater's "8-Bit Computer" video series several years ago, and I thought it would be a really interesting project to take on. Over the course of the majority of my career (and most of my hobbies), my understanding of "computer systems" has been primarily focused on *things above the hardware layer*. 

In my early days I spent some time building systems, servers, etc., but when *the cloud* came along, much of that was *abstracted away from me*. I have some minor experience programming in assembly, and good amount of experience with higher-level programming languages. I'm also comfortable explaining computer architecture at an academic or conceptual level, but what was missing was a deeper, hands-on, understanding on how a computer works at the hardware layer. Actually building an "A Register". Having the Control Unit tell a register to put information on the bus, and having RAM store that information for later use. Understanding how the CPU clock works at the electrical level. Understanding how each integrated circuit plays its part in each CPU module...and so on. 

What better way to *bridge these gaps* than to build a super-simple 8-bit system.

I did post a video to YouTube covering much of this build as well, that can be found here: [https://youtu.be/7FzZS5qhdJ4](https://youtu.be/7FzZS5qhdJ4])

## Credit Where Credit's Due

Full credit to Ben Eater, the schematics on his website, and his tutorial videos. His videos were the original inspiration to attempt this project, and without those, I a) probably wouldn't have even thought to attempt this; and b) even if I did, building it would have taken me orders of magnitude longer.

YouTube series: https://www.youtube.com/watch?v=HyznrdDSSGM

Website: https://eater.net/8bit (https://eater.net/)

## Table of Contents

[The Build](build/readme.md)

Parts I used, my thoughts, learnings, observations, "gotchas", reference images, architecture stuff, etc. as I worked through the build.

[Schematics](schematics/readme.md)

Mostly so that, at some point in the future, I can reference how I did any of this.

[Instruction Set Architecture (ISA)](isa.md)

The 'spec' for my instruction set. Instructions, registers, flags, execution cycle, etc. Capabilities of the system and hardware expectations.

[Microarchitecture](microarchitecture.md)

My implementation of the ISA, covering the Modules, Control Signals, and other behaviour (e.g. system reset).

[EEPROM Programming - Python](/python/readme.md#microcode-generation)

The Python program (`generate_microcode.py`) I used to convert my control signals into *firmware* for my EEPROMs.

[EEPROM Programming - Arduino](/arduino/readme.md#eeprom-programmer)

Actually getting the EEPROM *firmware* onto the chips.

[The Compiler](/python/readme.md#compiling-programs)

A look at the Python compiler I used (`compile.py`) to convert my assembly files to machine code.

[Program Execution](/arduino/readme.md#ram-programmer)

How I took a compiled program and loaded it onto the system, and ran it.

## Other

The datasheets I used for the project can be found in the [datasheets](datasheets/) folder.

## What's Next?

I have a lot of thoughts on what I could do *next* as an extension to this project, but I promised myself I'd *step away* from it for a month or two.

Ideas for *Phase 2*:
- I think when the weather gets warmer I may construct a *shadow box* to store this project in (a fun link to my woodworking hobby)
- I'd like to learn how to convert this, or pieces of this, into PCBs
- If I do it (or similar) again, I may try to make the following architectural changes:
    - Either a 16-bit Instruction Register with an 8-bit Opcode and 8-bit Operand, or two seperate registers
    - An additional Register, just because you can never have too many ;)
    - Three EEPROM chips to support up to (e.g.) 24 control lines
    - Potentially some sort of user input (but that would require a bunch of other things)