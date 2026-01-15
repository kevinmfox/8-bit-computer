/*
The SRAM needs to maintin power between programming and running.
The programmer needs to control the bus, and also expects access
to the following control signals: /RO, /MI, /RI
Additionally it must be able to control the clock (CLK).
Expected wiring:
* Arduino D2  > CLK
* Arduino D3  > /RI
* Arduino D4  > /MI
* Arduino D5  > /RO
* Arduino D6  > Bus bit0
* Arduino D7  > Bus bit1
* Arduino D8  > Bus bit2
* Arduino D9  > Bus bit3
* Arduino D10 > Bus bit4
* Arduino D11 > Bus bit5
* Arduino D12 > Bus bit6
* Arduino D13 > Bus bit7
*/

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "programs.h"

// ---------------- Pin mapping ----------------
// bus pins (LSB first)
const uint8_t BUS_PINS[8] = {6,7,8,9,10,11,12,13};

// control pins
const uint8_t RO_N_PIN = 5;   // /RO
const uint8_t MI_N_PIN = 4;   // /MI
const uint8_t RI_N_PIN = 3;   // /RI
const uint8_t CLK_PIN  = 2;   // clock pulse

// ---------------- timings ----------------
const unsigned int SETTLE_US    = 20;
const unsigned int CTRL_HOLD_US = 20;
const unsigned int CLK_PULSE_US = 50;
const unsigned int RAM_OUT_US   = 50;

static const uint8_t ORIGIN_ADDR = 0x00;  // where to load the program

// pad first N bytes to 0x00 (NOP)
static const uint16_t PAD_BYTES = 128;

// ---------------- bus helpers ----------------
void driveBus(uint8_t value) {
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(BUS_PINS[i], OUTPUT);
    digitalWrite(BUS_PINS[i], (value & (1 << i)) ? HIGH : LOW);
  }
  delayMicroseconds(SETTLE_US);
}

void releaseBus() {
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(BUS_PINS[i], INPUT);
    digitalWrite(BUS_PINS[i], LOW);
  }
  delayMicroseconds(SETTLE_US);
}

uint8_t readBus() {
  uint8_t v = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (digitalRead(BUS_PINS[i]) == HIGH) v |= (1 << i);
  }
  return v;
}

void pulseClock() {
  digitalWrite(CLK_PIN, HIGH);
  delayMicroseconds(CLK_PULSE_US);
  digitalWrite(CLK_PIN, LOW);
  delayMicroseconds(SETTLE_US);
}

// ---------------- MAR/RAM ops ----------------
// latches address to MAR
void setAddress(uint8_t addr) {
  driveBus(addr);
  digitalWrite(MI_N_PIN, LOW);
  delayMicroseconds(CTRL_HOLD_US);
  pulseClock();
  digitalWrite(MI_N_PIN, HIGH);
  delayMicroseconds(SETTLE_US);
}

// writes
void writeByte(uint8_t addr, uint8_t data) {
  // make sure RAM isn't trying to output
  digitalWrite(RO_N_PIN, HIGH); 

  // set the address on the MAR
  setAddress(addr);

  // put the data on the bus and set it in RAM
  driveBus(data);
  digitalWrite(RI_N_PIN, LOW);
  delayMicroseconds(CTRL_HOLD_US);
  pulseClock();
  digitalWrite(RI_N_PIN, HIGH);

  releaseBus();
}

uint8_t readByte(uint8_t addr) {
  // make sure RAM isn't writing
  digitalWrite(RI_N_PIN, HIGH);

  // set the address on the MAR
  setAddress(addr);

  // tell RAM to output and read the bus
  releaseBus();
  digitalWrite(RO_N_PIN, LOW);
  delayMicroseconds(RAM_OUT_US);

  uint8_t v = readBus();

  digitalWrite(RO_N_PIN, HIGH);
  delayMicroseconds(SETTLE_US);
  return v;
}

// ---------------- Utilities ----------------
void dumpRam(uint8_t startAddr, uint8_t count) {
  Serial.print(F("Dump 0x"));
  if (startAddr < 16) Serial.print('0');
  Serial.print(startAddr, HEX);
  Serial.print(F(".. count "));
  Serial.println(count);

  for (uint8_t i = 0; i < count; i++) {
    uint8_t addr = startAddr + i;
    uint8_t v = readByte(addr);

    Serial.print(F("0x"));
    if (addr < 16) Serial.print('0');
    Serial.print(addr, HEX);
    Serial.print(F(": 0x"));
    if (v < 16) Serial.print('0');
    Serial.println(v, HEX);
  }
}

bool programAndVerifyPadded(const uint8_t* prog, uint8_t origin, uint8_t len, uint16_t padBytes) {
  Serial.print(F("Padding first "));
  Serial.print(padBytes);
  Serial.println(F(" bytes to 0x00 (NOP)..."));

  // pad region
  for (uint16_t a = 0; a < padBytes; a++) {
    writeByte((uint8_t)a, 0x00);
  }

  Serial.println(F("Verifying pad..."));
  uint16_t padBad = 0;
  for (uint16_t a = 0; a < padBytes; a++) {
    uint8_t got = readByte((uint8_t)a);
    if (got != 0x00) padBad++;
  }

  Serial.print(F("Pad verify bad bytes: "));
  Serial.println(padBad);

  // program region
  Serial.print(F("Programming RAM @ origin 0x"));
  if (origin < 16) Serial.print('0');
  Serial.print(origin, HEX);
  Serial.print(F(" len="));
  Serial.println(len);

  for (uint8_t i = 0; i < len; i++) {
    uint8_t b = pgm_read_byte(prog + i);
    writeByte((uint8_t)(origin + i), b);
  }

  Serial.println(F("Write complete. Verifying program..."));

  uint8_t bad = 0;
  for (uint8_t i = 0; i < len; i++) {
    uint8_t expected = pgm_read_byte(prog + i);
    uint8_t addr = (uint8_t)(origin + i);
    uint8_t got = readByte(addr);

    if (got != expected) {
      bad++;
      Serial.print(F("Mismatch @0x"));
      if (addr < 16) Serial.print('0');
      Serial.print(addr, HEX);
      Serial.print(F(" exp 0x"));
      if (expected < 16) Serial.print('0');
      Serial.print(expected, HEX);
      Serial.print(F(" got 0x"));
      if (got < 16) Serial.print('0');
      Serial.println(got, HEX);
    }
  }

  Serial.print(F("Program verify done. Bad bytes: "));
  Serial.println(bad);

  return (padBad == 0) && (bad == 0);
}

// Read an integer selection (e.g., "12") from Serial
int readSelectionInt() {
  while (!Serial.available()) { /* wait */ }
  String s = Serial.readStringUntil('\n');
  s.trim();
  if (s.length() == 0) return -1;
  int n = s.toInt();
  return n;
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);

  pinMode(RO_N_PIN, OUTPUT);
  pinMode(MI_N_PIN, OUTPUT);
  pinMode(RI_N_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);

  digitalWrite(RO_N_PIN, HIGH);
  digitalWrite(MI_N_PIN, HIGH);
  digitalWrite(RI_N_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);

  releaseBus();

  Serial.println();
  Serial.println(F("In-circuit RAM programmer."));
  Serial.println(F("Ensure CPU clock is stopped and no other outputs drive the bus."));
  Serial.println();

  for (uint8_t i = 0; i < PROGRAM_COUNT; i++) {
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(PROGRAMS[i].name);
    Serial.print(" (len=");
    Serial.print(PROGRAMS[i].len);
    Serial.println(")");
  }
  Serial.println();
  Serial.println(F("Select program number and press Enter:"));

  int sel1based = readSelectionInt();
  int sel = sel1based - 1;

  if (sel < 0 || sel >= (int)PROGRAM_COUNT) {
    Serial.println(F("Invalid selection."));
    return;
  }

  Serial.print(F("Selected: "));
  Serial.println(PROGRAMS[sel].name);

  bool ok = programAndVerifyPadded(PROGRAMS[sel].bytes, ORIGIN_ADDR, PROGRAMS[sel].len, PAD_BYTES);

  // dumpRam(0x00, 32);

  Serial.println(ok ? F("SUCCESS. Disconnect Arduino board, switch to RUN, reset CPU, start main clock.")
                    : F("FAIL. Fix something and retry."));
}

void loop() {}
