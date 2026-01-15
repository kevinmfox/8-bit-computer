/*
This sketch sets up the 1602 LCD to display information from the 8-bit bus.
It has four 4 display modes:
* Binary  - e.g. BIN: 10101010
* Decimal - e.g. DEC:      252
* Hex     - e.g. HEX:       B3
* All     - combines all the above
* Secret  - Randomly selects a message :)
It expects the CPU to tell it to pull/latch from the bus via DISPLAY_OUT_PIN.
Primarily for debugging, there's also a BUS_MODE_PIN that, if set LOW, will constantly pull from the bus.
*/

#include <LiquidCrystal.h>

// initialize LCD PINs > Arduino PINs: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(10, 9, 4, 5, 6, 7);

// data will come from the bus to be displayed on the LCD
// bus input pins
const uint8_t BUS_PINS[8] = {
  A0, // bus 0
  A1, // bus 1
  A2, // bus 2
  A3, // bus 3
  A4, // bus 4
  A5, // bus 5
  3,  // bus 6
  2   // bus 7
};

// mode button (hex, decimal, binary, all)
const uint8_t DISPLAY_MODE_PIN = 8;

// display output enable latch - sent from the CPU
const uint8_t DISPLAY_OUT_PIN = 11;

// bus mode pin (live or latch)
// HIGH = latch mode, LOW = live mode
const uint8_t BUS_MODE_PIN = 13;

// display modes
enum DisplayMode {
  MODE_BINARY = 0,
  MODE_DECIMAL = 1,
  MODE_HEX = 2,
  MODE_ALL = 3,
  MODE_SECRET = 4
};

// set the starting display mode
// Note: I set mine to hex as, upon booting, my board would send a low signal
//  and flip the mode to All...and I was too lazy to fix it in hardware :)
DisplayMode currentMode = MODE_HEX;

// debounced button state
uint8_t lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

// edge-detect for latch signal (DISPLAY_OUT_PIN)
uint8_t lastDisplayOutState = HIGH;

// used for some secret messaging
bool SECRETS_ENABLED = true;
uint8_t secretValue = 0;
bool secretGenerated = false;
const char* secretMessages[] = {
  "I love Danielle",
  "I love Theo",
  "I love Alex",
};

// bus values
uint8_t latchedBusValue = 0;   // value captured in latch mode
uint8_t liveBusValue = 0;      // latest sampled value in live mode
uint8_t displayValue = 0;      // value currently shown on LCD

// mode-change flag so we refresh when format changes
bool modeChanged = true;

// live-mode sampling throttle
const unsigned long liveUpdateInterval = 100; // ms
unsigned long lastLiveSampleTime = 0;

// ---------- FUNCTIONS ----------

void handleModeButton() {
  int reading = digitalRead(DISPLAY_MODE_PIN);

  // if the state changed, restart the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // if stable long enough, treat as real state
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // edge detection on a debounced state
    static uint8_t stableState = HIGH;

    if (reading != stableState) {
      stableState = reading;

      // button pressed - switch modes
      if (stableState == LOW) {
        if (currentMode == MODE_BINARY) {
          currentMode = MODE_DECIMAL;
        } else if (currentMode == MODE_DECIMAL) {
          currentMode = MODE_HEX;
        } else if (currentMode == MODE_HEX) {
          currentMode = MODE_ALL;
        } else if (currentMode == MODE_ALL) {
          if (SECRETS_ENABLED) {
            secretGenerated = false;
            currentMode = MODE_SECRET;
          } else {
            currentMode = MODE_BINARY;
          }
        } else if (currentMode == MODE_SECRET) {
          currentMode = MODE_BINARY;
        }

        modeChanged = true;
      }
    }
  }

  lastButtonState = reading;
}

void handleDisplayOutSignal() {
  uint8_t displayOutState = digitalRead(DISPLAY_OUT_PIN);

  // detect rising edge: HIGH --> LOW
  if (lastDisplayOutState == HIGH && displayOutState == LOW) {
    latchedBusValue = readBus();
  }

  lastDisplayOutState = displayOutState;
}

void handleBusLiveSampling() {
  unsigned long now = millis();
  
  // make sure we're at or past our update interval
  if (now - lastLiveSampleTime < liveUpdateInterval) {
    return;
  }

  lastLiveSampleTime = now;
  liveBusValue = readBus();
}

// get the values from the bus
uint8_t readBus() {
  uint8_t value = 0;
  for (uint8_t bit = 0; bit < 8; bit++) {
    int pinState = digitalRead(BUS_PINS[bit]);
    if (pinState == HIGH) {
      value |= (1 << bit);
    }
  }
  return value;
}

void updateDisplay(uint8_t value) {
  lcd.clear();

  char line0[17];
  char line1[17];

  // create the binary string
  char binStr[9];
  for (int i = 7; i >= 0; i--) {
    binStr[7 - i] = (value & (1 << i)) ? '1' : '0';
  }
  binStr[8] = '\0';

  switch (currentMode) {
    case MODE_BINARY: {
      snprintf(line0, sizeof(line0), "BIN:    %s", binStr);
      snprintf(line1, sizeof(line1), "");
      break;
    }

    case MODE_DECIMAL:
      snprintf(line0, sizeof(line0), "DEC:%12u", value);
      snprintf(line1, sizeof(line1), "");
      break;

    case MODE_HEX:
      snprintf(line0, sizeof(line0), "HEX:%8s0x%02X", "", value);
      snprintf(line1, sizeof(line1), "");
      break;

    case MODE_ALL:
      snprintf(line0, sizeof(line0), "D:%4u   H: 0x%02X", value, value);
      snprintf(line1, sizeof(line1), "B:      %s", binStr);
      break;

    // you can have the display randomly show some phrases - just for fun
    case MODE_SECRET:
      if (!secretGenerated) {
        const uint8_t NUM_SECRETS = sizeof(secretMessages) / sizeof(secretMessages[0]);
        secretValue = random(NUM_SECRETS);
        secretGenerated = true;
      }

      snprintf(line0, sizeof(line0), "%s", secretMessages[secretValue]);
      snprintf(line1, sizeof(line1), "");
      break;
  }

  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);
}

// ---------- SETUP ----------

void setup() {
  randomSeed(analogRead(A6));

  // bus mode pin: default HIGH (latch mode) via pullup
  pinMode(BUS_MODE_PIN, INPUT_PULLUP);

  // bus pins as inputs
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(BUS_PINS[i], INPUT);
  }

  // mode button with pullup
  pinMode(DISPLAY_MODE_PIN, INPUT_PULLUP);

  // display out pin
  pinMode(DISPLAY_OUT_PIN, INPUT_PULLUP);
  lastDisplayOutState = digitalRead(DISPLAY_OUT_PIN);
  
  // initialize and clear the display
  lcd.begin(16, 2);
  lcd.clear();

  // show a brief message before showing data: 8-bit Computer :)
  lcd.setCursor(0, 0);
  lcd.print("8-bit Computer");
  lcd.setCursor(0, 1);
  lcd.print(":)");
  delay(1000);

  // initialize button debounce state from actual pin
  lastButtonState = digitalRead(DISPLAY_MODE_PIN);
  lastDebounceTime = millis();

  // initial bus values
  uint8_t initial = readBus();
  latchedBusValue = initial;
  liveBusValue = initial;
  displayValue = initial;

  updateDisplay(displayValue);
}

// ---------- MAIN LOOP ----------

void loop() {
  // handle mode (BIN/DEC/HEX/ALL/SECRET) button
  handleModeButton();

  // HIGH = latch mode, LOW = live mode
  bool latchMode = (digitalRead(BUS_MODE_PIN) == HIGH);

  if (latchMode) {
    handleDisplayOutSignal();
  } else {
    handleBusLiveSampling();
  }

  // decide which value should be displayed
  uint8_t desiredValue = latchMode ? latchedBusValue : liveBusValue;

  // only redraw if value changed or mode changed
  if (desiredValue != displayValue || modeChanged) {
    displayValue = desiredValue;
    updateDisplay(displayValue);
    modeChanged = false;
  }

  delay(1);
}
