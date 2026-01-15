# Tooling Schematics

Both the EEPROM and RAM programmer used an Arduino Uno for programming.

More detailed requirements can be found in the respective `arduino` programs. These sketches simply outline how I programmed these chips.

## EEPROM Programmer

Used the sketch found in [arduino/eeprom_programmer](../../arduino/eeprom_programmer).

Each EEPROM chip was programmed individually (i.e. remove chip from board; program it; put it back; repeat).

## RAM Programmer

Used the sketch found in [arduino/ram_programmer](../../arduino/ram_programmer).

The Arduino was hooked up directly to the project via the bus and several control lines (`CLK`, `/MI`, `/RI`, `/RO`).

It's important that the Arduino is the only thing driving the bus, that the Control Unit is 'disconnected', and the Clock isn't running. In my build, I used a toggle switch to *disconnect* the Control Unit and stop the Clock.