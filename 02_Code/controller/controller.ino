#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <SevSeg.h>

const int PIN_ERROR_LED_ONE = 8;
const int PIN_ERROR_LED_TWO = 9;

// Buzzer
const int PIN_BUZZER = 10;
const int TONE_EVEN = 1046;
const int TONE_ODD = 1174;
const int TONE_DURATION = 30;

// Clock
const byte PIN_CLOCK_SEGMENTS[] = {11, 12, 13, 14, 15, 16, 17, 18};
const byte PIN_CLOCK_DIGITS[] = {19, 20, 21, 22};
const int PIN_TIME_PULSE_BUS = 30;
SevSeg clockSevSeg;

// Debug
const int PIN_ERROR_TRIGGER = 46;
int errorTriggerState = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output 
int buttonState;
int lastButtonState = LOW;

// Time
long currentTime = 90000;
long lastMillis = 0;
int timeMultiplier = 10;
int timeInSeconds = 0;

// Errors
int errorCount = 0;



void setup() {
  pinMode(PIN_ERROR_LED_ONE, OUTPUT);
  pinMode(PIN_ERROR_LED_TWO, OUTPUT);
  pinMode(PIN_ERROR_TRIGGER, INPUT);
  pinMode(PIN_TIME_PULSE_BUS, OUTPUT);
  Serial.begin(9600); // Debugging
  setupSevSeg();
  displayRemainingTime();
  clockSevSeg.refreshDisplay();
  delay(2000); // delay start by 2 secs
  lastMillis = millis();
}

void loop() {
  readErrorTrigger();
  displayRemainingTime();
  clockSevSeg.refreshDisplay();
}

void setupSevSeg() {
  clockSevSeg.begin(COMMON_CATHODE, 4, PIN_CLOCK_DIGITS, PIN_CLOCK_SEGMENTS, false, false, true, false);
  clockSevSeg.setBrightness(90);
}

void displayRemainingTime() {
  if (currentTime >= 0) {
    long currentMillis = millis();
    long loopTime = currentMillis - lastMillis;
    if (loopTime < 10) {
      return;
    }
    int elapsedGameTime = loopTime * timeMultiplier / 10;
    currentTime = currentTime - elapsedGameTime;
    lastMillis = currentMillis;

    int newTimeInSeconds = (currentTime - (currentTime % 1000)) / 1000;
    int minutes = (newTimeInSeconds - (newTimeInSeconds % 60)) / 60;
    int seconds = newTimeInSeconds - (minutes * 60);
    int milliseconds = currentTime - (((minutes * 60) + seconds) * 1000);
    int centiseconds = (milliseconds - (milliseconds % 10)) / 10;
    if (currentTime > 60000) {
      int decimalPoint = 2;
      if (newTimeInSeconds % 2 == 0) {
        decimalPoint = -1;
      }
      clockSevSeg.setNumber((minutes * 100) + seconds, decimalPoint);
    } else {
      clockSevSeg.setNumber((seconds * 100) + centiseconds, 2);
    }
    if (newTimeInSeconds != timeInSeconds) {
      timeInSeconds = newTimeInSeconds;
      clockBeep();
    }
  }
}

void clockBeep() {
  if (timeInSeconds % 2 == 0) {
    tone(PIN_BUZZER, TONE_EVEN, TONE_DURATION);
  } else {
    tone(PIN_BUZZER, TONE_ODD, TONE_DURATION);
  }
  digitalWrite(PIN_TIME_PULSE_BUS, HIGH);
  delay(1);
  digitalWrite(PIN_TIME_PULSE_BUS, LOW);
}

void triggerError() {
  errorCount++;
  if (errorCount == 1) {
    timeMultiplier = 12;
    digitalWrite(PIN_ERROR_LED_ONE, HIGH);
  }
  if (errorCount == 2) {
    timeMultiplier = 15;
    digitalWrite(PIN_ERROR_LED_TWO, HIGH);
  }
  if (errorCount >= 3) {
    timeMultiplier = 0;
  }
}

void readErrorTrigger() {
  int reading = digitalRead(PIN_ERROR_TRIGGER);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        triggerError();
      }
    }
  }
  lastButtonState = reading;
}