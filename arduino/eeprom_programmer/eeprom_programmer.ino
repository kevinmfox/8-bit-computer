/*
This sketch programs the EEPROMs (AT28C64B).
You'll be prompted to select which chip to program.

Assumptions:
* You've already generated microcode_images.h via generate_microcode.py.
* You're using two 74-595 shift registers for programming.

The EEPROMs were removed from the project and programmed individually, then
put back into the project.
Expected wiring:
* Arduino D2  > EEPROM PIN 11 (DQ0)
* Arduino D3  > EEPROM PIN 12 (DQ1)
* Arduino D4  > EEPROM PIN 13 (DQ2)
* Arduino D5  > EEPROM PIN 15 (DQ3)
* Arduino D6  > EEPROM PIN 16 (DQ4)
* Arduino D7  > EEPROM PIN 17 (DQ5)
* Arduino D8  > EEPROM PIN 18 (DQ6)
* Arduino D9  > EEPROM PIN 19 (DQ7)
* Arduino D10 > 595-LSB PIN 14 (SER)
* Arduino D11 > 595-* PIN 11 (SRCK) 
* Arduino D12 > 595-* PIN 12 (RCK)
* Arduino A0  > EEPROM PIN 27 (/WE)
* Arduino A1  > EEPROM PIN 22 (/OE)
*/

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "microcode_images.h"   // this is generated automatically via generate_microcode.py

// ---------------- 74HC595 PINs ----------------
const int PIN_SER  = 10;  // PIN 14 - SER
const int PIN_SRCK = 11;  // PIN 11 - SRCK
const int PIN_RCK  = 12;  // PIN 12 - RCK

// ---------------- EEPROM PINs ----------------
const int PIN_WE = A0;    // PIN 27 - /WE
const int PIN_OE = A1;    // PIN 22 - /OE

// ---------------- EEPROM Data PINs ----------------
const int DATA_PINS[8] = {2,3,4,5,6,7,8,9};

// ---------------- Timing ----------------
static const unsigned int ADDR_SETTLE_US = 2;
static const unsigned int DATA_SETTLE_US = 2;
static const unsigned int WE_PULSE_US    = 3;
static const unsigned int WRITE_CYCLE_MS = 10;

// retries per address
static const int WRITE_RETRIES = 3;

// ---------------- data bus helpers ----------------
static inline void setDataBusOutput() { for (int i=0;i<8;i++) pinMode(DATA_PINS[i], OUTPUT); }
static inline void setDataBusInput()  { for (int i=0;i<8;i++) pinMode(DATA_PINS[i], INPUT);  }

static inline void writeDataBus(uint8_t v){
  for (int i=0;i<8;i++) digitalWrite(DATA_PINS[i], (v >> i) & 1);
}

static inline uint8_t readDataBus(){
  uint8_t v = 0;
  for (int i=0;i<8;i++) if (digitalRead(DATA_PINS[i])) v |= (1 << i);
  return v;
}

static void printBits(uint8_t v) {
  for (int i = 7; i >= 0; i--) Serial.print((v >> i) & 1);
}

static void printDiff(uint8_t expected, uint8_t got) {
  Serial.print("  exp 0b"); printBits(expected);
  Serial.print(" (0x"); if (expected < 16) Serial.print('0'); Serial.print(expected, HEX); Serial.println(")");
  Serial.print("  got 0b"); printBits(got);
  Serial.print(" (0x"); if (got < 16) Serial.print('0'); Serial.print(got, HEX); Serial.println(")");
}

void setAddress(uint16_t addr){
  uint8_t low  = (uint8_t)(addr & 0xFF);
  uint8_t bit8 = (uint8_t)((addr >> 8) & 0x01);

  uint8_t high = 0;
  high |= (bit8 << 1); // QB

  // be safe while changing addresses
  digitalWrite(PIN_WE, HIGH);
  digitalWrite(PIN_OE, HIGH);

  digitalWrite(PIN_RCK, LOW);
  shiftOut(PIN_SER, PIN_SRCK, MSBFIRST, high); // 595 #2
  shiftOut(PIN_SER, PIN_SRCK, MSBFIRST, low);  // 595 #1
  digitalWrite(PIN_RCK, HIGH);

  delayMicroseconds(ADDR_SETTLE_US);
}

static inline void pulseWE(){
  digitalWrite(PIN_WE, LOW);
  delayMicroseconds(WE_PULSE_US);
  digitalWrite(PIN_WE, HIGH);
}

void writeEEPROM(uint16_t addr, uint8_t data){
  digitalWrite(PIN_OE, HIGH);  // disable output during write
  digitalWrite(PIN_WE, HIGH);

  setAddress(addr);

  setDataBusOutput();
  writeDataBus(data);
  delayMicroseconds(DATA_SETTLE_US);

  pulseWE();
  delay(WRITE_CYCLE_MS);
}

uint8_t readEEPROM(uint16_t addr){
  digitalWrite(PIN_WE, HIGH);
  setAddress(addr);

  setDataBusInput();
  digitalWrite(PIN_OE, LOW);
  delayMicroseconds(DATA_SETTLE_US);

  uint8_t v = readDataBus();

  digitalWrite(PIN_OE, HIGH);
  return v;
}

bool writeAndVerify(uint16_t addr, uint8_t data, int retries=WRITE_RETRIES){
  for(int attempt=1; attempt<=retries; attempt++){
    writeEEPROM(addr, data);
    uint8_t got = readEEPROM(addr);
    if(got == data) return true;

    Serial.print("Mismatch @0x");
    Serial.print(addr, HEX);
    Serial.print(" attempt ");
    Serial.println(attempt);
    printDiff(data, got);
  }
  return false;
}

const uint8_t* chooseImageByChar(char c){
  if (c == '1') return EEPROM1_IMAGE;
  if (c == '2') return EEPROM2_IMAGE;
  return nullptr;
}

/*
this test detects:
* stuck bits (always 1 or always 0)
* swapped data lines (walking-bit comes back on the wrong bit)
* weak/float lines (intermittent mismatch)
*/
bool dataBusSelfTest(uint16_t addr = 0x000){
  Serial.println("Pre-flight test - walking 1s and walking 0s...");
  bool ok = true;

  // walking 1s
  for (uint8_t bit = 0; bit < 8; bit++) {
    uint8_t v = (uint8_t)(1u << bit);
    if(!writeAndVerify(addr, v, 2)){
      Serial.print("DQ test FAIL (walking 1) bit ");
      Serial.println(bit);
      ok = false;
    }
  }

  // walking 0s
  for (uint8_t bit = 0; bit < 8; bit++) {
    uint8_t v = (uint8_t)~(1u << bit);
    if(!writeAndVerify(addr, v, 2)){
      Serial.print("DQ test FAIL (walking 0) bit ");
      Serial.println(bit);
      ok = false;
    }
  }

  Serial.println(ok ? "PASS" : "FAIL");
  return ok;
}

bool programImage(const uint8_t* imageProgmem){
  const uint16_t SIZE = 512;

  Serial.println("Programming...");
  for(uint16_t addr=0; addr<SIZE; addr++){
    uint8_t expected = pgm_read_byte_near(imageProgmem + addr);

    if(!writeAndVerify(addr, expected, WRITE_RETRIES)){
      Serial.print("HARD FAIL: 0x");
      Serial.println(addr, HEX);
      return false;
    }

    // write status every 64 bytes
    if ((addr & 0x3F) == 0) {
      Serial.print("  Wrote and verified up to: 0x");
      Serial.println(addr, HEX);
    }
  }
  Serial.println("Write complete.");
  return true;
}

bool finalVerify(const uint8_t* imageProgmem){
  const uint16_t SIZE = 512;
  Serial.println("Final verification pass...");
  uint16_t bad = 0;

  for(uint16_t addr=0; addr<SIZE; addr++){
    uint8_t expected = pgm_read_byte_near(imageProgmem + addr);
    uint8_t got      = readEEPROM(addr);

    if(got != expected){
      bad++;
      if (bad <= 20) {
        Serial.print("Mismatch: 0x");
        Serial.println(addr, HEX);
        printDiff(expected, got);
      }
    }
  }

  Serial.print("Final verification done. Bad bytes: ");
  Serial.println(bad);
  return bad == 0;
}

void setup(){
  Serial.begin(115200);

  // setup some 595 PINs
  pinMode(PIN_SER, OUTPUT);
  pinMode(PIN_SRCK, OUTPUT);
  pinMode(PIN_RCK, OUTPUT);
  
  // setup some EEPROM PINs
  pinMode(PIN_WE, OUTPUT);
  pinMode(PIN_OE, OUTPUT);
  digitalWrite(PIN_WE, HIGH);
  digitalWrite(PIN_OE, HIGH);

  // start with bus as input to be safe
  setDataBusInput();

  // ask the user what they want to burn
  Serial.println();
  Serial.println("AT28C64B microcode programmer.");
  Serial.println("Send '1' to burn EEPROM #1 or '2' to burn EEPROM #2.");
  while (!Serial.available()) { /* wait */ }
  char sel = Serial.read();

  // make sure the selection is valid
  const uint8_t* img = chooseImageByChar(sel);
  if (!img) {
    Serial.println("Invalid selection. Send '1' or '2'.");
    return;
  }

  Serial.print("Selected image: ");
  Serial.println(sel);

  // pre-flight testing
  if(!dataBusSelfTest(0x000)){
    Serial.println("Aborting due to data bus failures.");
    return;
  }

  // program the image
  if(!programImage(img)){
    Serial.println("Programming failed. Fix something and retry.");
    return;
  }

  bool ok = finalVerify(img);
  Serial.println(ok ? "SUCCESS. Image programmed and verified."
                    : "FAIL. Final verify found issues.");
}

void loop(){}
