#include <Arduino.h>

// =====================================================
// Concurrent Stair Lighting with Dynamic Overlap and Extended Wait:
// - Lights are turned on by concurrent sensor triggers.
// - After all lights are on (WAIT_ON), the OFF sequence begins using the direction
//   indicated by the second-last sensor trigger.
// - During TURNING_OFF, if a sensor is triggered:
//    • If its on-direction is opposite to the off-direction, cancel OFF and resume ON.
//    • If its on-direction is the same as the off-direction, continue OFF while starting ON concurrently.
// - Each sensor trigger (even during WAIT_ON) resets the lights-on timer, extending the wait.
// - A relay counter is used so overlapping commands don’t conflict.
// =====================================================

// ----- State Machine Definitions -----
enum SystemPhase { 
  IDLE,           // waiting for any sensor trigger
  TURNING_ON,     // turning on relays sequentially (can be concurrent from top and/or bottom)
  WAIT_ON,        // all relays on; waiting before starting OFF sequence (extended by sensor triggers)
  TURNING_OFF,    // turning off relays sequentially in chosen direction
  TURNING_OFF_WITH_ON  // off sequence continues, but an on sequence is started concurrently
};
SystemPhase systemPhase = IDLE;

// ----- Hardware Pin Definitions -----
const int sensorTopPin = 34;     // sensor signals (assumed to be 3.3V safe)
const int sensorBottomPin = 35;
// 15 relay channels; we assume index 0 is the “top” step, index 14 is the “bottom”
const int relayPins[15] = {4, 5, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 33};

// ----- Timing Settings (in ms) -----
const unsigned long stepDelay = 500;       // delay between each relay action
const unsigned long lightsOnDuration = 2000; // duration to keep lights on (this will be extended with each sensor trigger)

// ----- Sensor Debounce Settings -----
const unsigned long debounceDelay = 50;  // Debounce delay in milliseconds

bool lastTopReading = LOW;
bool stableTopSignal = LOW;
unsigned long lastTopDebounceTime = 0;

bool lastBottomReading = LOW;
bool stableBottomSignal = LOW;
unsigned long lastBottomDebounceTime = 0;

// ----- Global Variables for ON Sequences -----
bool topActive = false;    // if top sensor is active (i.e. turning on from top)
bool bottomActive = false; // if bottom sensor is active
int topIndex = 0;        // for top on sequence (from index 0 upward)
int bottomIndex = 14;    // for bottom on sequence (from index 14 downward)
unsigned long topLastStepTime = 0;
unsigned long bottomLastStepTime = 0;

// ----- Global Variables for OFF Sequence -----
int offDirection = 0; // 0 = off from top-to-bottom, 1 = off from bottom-to-top
unsigned long offLastStepTime = 0;

// ----- Relay Counter Array -----
// For each relay, a counter > 0 means the relay should be ON.
bool relayCounter[15] = {0};

// ----- Sensor Trigger Times -----
// Updated continuously so we can choose the second-last sensor.
unsigned long topTriggerTime = 0;
unsigned long bottomTriggerTime = 0;

// ----- WAIT_ON Timing -----
unsigned long waitOnStartTime = 0;

// ----- Helper Functions for Relay Control -----
// void updateRelayOutput(int idx) {
//   // Drive relay HIGH if counter > 0, LOW otherwise.
//   digitalWrite(relayPins[idx], (relayCounter[idx] > 0) ? HIGH : LOW);
// }

void relayTurnOn(int idx) {
  if (relayCounter[idx] == false) {
    relayCounter[idx] = true;
  }
  digitalWrite(relayPins[idx], HIGH);
}

void relayTurnOff(int idx) {
  if (relayCounter[idx] == true) {
    relayCounter[idx] = false;
  }
  digitalWrite(relayPins[idx], LOW);
}

// ----- Function to Reset System Variables for a New Cycle -----
void resetSystem() {
  systemPhase = IDLE;
  topActive = false;
  bottomActive = false;
  topIndex = 0;
  bottomIndex = 14;
  topTriggerTime = 0;
  bottomTriggerTime = 0;
  for (int i = 0; i < 15; i++) {
    relayCounter[i] = 0;
    relayTurnOff(i);
  }
  Serial.println("Cycle complete. System reset to IDLE.");
}

// =====================================================
// Main Setup and Loop
// =====================================================
void setup() {
  Serial.begin(115200);
  pinMode(sensorTopPin, INPUT);
  pinMode(sensorBottomPin, INPUT);
  for (int i = 0; i < 15; i++) {
    pinMode(relayPins[i], OUTPUT);
    relayCounter[i] = 0;      // Ensure all start off
    relayTurnOff(i);
  }
  resetSystem();
  }


void loop() {

  unsigned long currentTime = millis();

  // Read current raw values.
  bool topSignal = digitalRead(sensorTopPin);
  bool bottomSignal = digitalRead(sensorBottomPin);

  // Process top sensor debounce.
  if (topSignal != lastTopReading) {
    lastTopDebounceTime = currentTime;
  }
  if ((currentTime - lastTopDebounceTime) >= debounceDelay) {
    if (topSignal != stableTopSignal) {
      stableTopSignal = topSignal;
      // Only update trigger time on a rising edge.
      if (stableTopSignal == HIGH) {
        topTriggerTime = currentTime;
        if (systemPhase != WAIT_ON) {
          topActive = true;
        }
      }
    }
  }
  lastTopReading = topSignal;

  // Process bottom sensor debounce.
  if (bottomSignal != lastBottomReading) {
      lastBottomDebounceTime = currentTime;
  }
  if ((currentTime - lastBottomDebounceTime) >= debounceDelay) {
    if (bottomSignal != stableBottomSignal) {
      stableBottomSignal = bottomSignal;
      // Only update trigger time on a rising edge.
      if (stableBottomSignal == HIGH) {
        bottomTriggerTime = currentTime;
        if (systemPhase != WAIT_ON) {
          bottomActive = true;
        }
      }
    }
  }
  lastBottomReading = bottomSignal;

  // SENSOR DETECTION
  if (systemPhase == IDLE) {
    if (topActive || bottomActive) {
      systemPhase = TURNING_ON;
      if (topActive) {
        topIndex = 0;
      }
      if (bottomActive) {
        bottomIndex = 14;
      }
    }
  }
  if (systemPhase == WAIT_ON) {
    topActive = false;
    bottomActive = false;
    if (stableTopSignal == HIGH || stableBottomSignal == HIGH) {
      waitOnStartTime = currentTime;
    }
    if ((currentTime - waitOnStartTime) >= lightsOnDuration) {
      systemPhase = TURNING_OFF;
      if (topTriggerTime < bottomTriggerTime) {
        offDirection = 0;
      } else {
        offDirection = 1;
      }
    }
    bottomIndex = offDirection == 0 ? 0 : 14;
    topIndex = offDirection == 0 ? 0 : 14;
  }
  if (systemPhase == TURNING_OFF) {
    if (offDirection == 1) {
      if (topActive) {
        systemPhase = TURNING_ON;
      }
      else if (bottomActive) {
        systemPhase = TURNING_OFF_WITH_ON;
      }
    }
    else if (offDirection == 0) {
      if (bottomActive) {
        systemPhase = TURNING_ON;
      }
      else if (topActive) {
        systemPhase = TURNING_OFF_WITH_ON;
      }
    }
  }

    // STATE MACHINE PROCESSING
  if (systemPhase == TURNING_ON) {
    if (topActive) {
      if (currentTime - topLastStepTime >= stepDelay) {
        if (topIndex < 15) {
          relayTurnOn(topIndex);
          topLastStepTime = currentTime;
          topIndex++;
        }
        if (topIndex == 15) {
          systemPhase = WAIT_ON;
          waitOnStartTime = currentTime;
          topTriggerTime = currentTime;
        }
      }
    }
    if (bottomActive) {
      if (currentTime - bottomLastStepTime >= stepDelay) {
        if (bottomIndex >= 0) {
          relayTurnOn(bottomIndex);
          bottomLastStepTime = currentTime;
          bottomIndex--;
        }
        if (bottomIndex == -1) {
          systemPhase = WAIT_ON;
          waitOnStartTime = currentTime;
          bottomTriggerTime = currentTime;
        }
      }
    }
  }

  if (systemPhase == TURNING_OFF) {
    if (offDirection == 0) {
      if (bottomIndex < 15) {
        if ((currentTime - offLastStepTime) >= stepDelay) {
          relayTurnOff(bottomIndex);
          offLastStepTime = currentTime;
          bottomIndex++;
        }
      }
      if (bottomIndex == 15) {
        resetSystem();
      }
    }
    if (offDirection == 1) {
      if (topIndex >= 0) {
        if ((currentTime - offLastStepTime) >= stepDelay) {
          relayTurnOff(topIndex);
          offLastStepTime = currentTime;
          topIndex--;
        }
      }
      if (topIndex == -1) {
        resetSystem();
      }
    }
  }
  if (systemPhase == TURNING_OFF_WITH_ON) {
    if (offDirection == 0) {
      if (!bottomActive) {
        if (bottomIndex < 15) {
          if ((currentTime - offLastStepTime) >= stepDelay) {
            relayTurnOff(bottomIndex);
            offLastStepTime = currentTime;
            bottomIndex++;
          }
          if ((currentTime - topLastStepTime) >= stepDelay) {
            relayTurnOn(topIndex);
            topLastStepTime = currentTime;
            topIndex++;
          }
        }
        else {
          systemPhase = TURNING_ON;
        }
      }
      else {
        systemPhase = TURNING_ON;
      }
    }
    if (offDirection == 1) {
      if (!topActive) {
        if (topIndex >= 0) {
          if ((currentTime - offLastStepTime) >= stepDelay) {
            relayTurnOff(topIndex);
            offLastStepTime = currentTime;
            topIndex--;
          }
          if ((currentTime - bottomLastStepTime) >= stepDelay) {
            relayTurnOn(bottomIndex);
            bottomLastStepTime = currentTime;
            bottomIndex--;
          }
        }
        else {
          systemPhase = TURNING_ON;
        }
      }
      else {
        systemPhase = TURNING_ON;
      }
    }
  }

}