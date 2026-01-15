# Main Schematics

Once you load the `main.kicad_pro` file in KiCad, it's easy enough to see the main layout and click through the various module schematics.

While I made efforts to align my schematics to what I actually built, there is some variation. This was usually caused by either space constraints or moving something to a location where it would *make more sense*. A few examples: 

- The build has the Status Lights (Red = Halt; Green = Running) on the Memory Address Register board.
- The physical ALU spans across two horizontal breadboard, where my schematic may imply otherwise.
- The build has the `HLT` and `SU` decoding on the Microstep Counter board (there was just more room) vs. the Instruction Register.
- The build has a completely separate board for some *auxiliary* chips used by the Control Unit due to space constraints, and it also holds the `74LS173` chip for the `SU` Flag. I also used the `74HC14` inverter on this board to help with other inversions.
- The Control Unit spans across two breadboards (chips & LEDs).