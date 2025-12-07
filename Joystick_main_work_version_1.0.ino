#include <MIDI.h>
#include <EEPROM.h>
#include <elapsedMillis.h>

MIDI_CREATE_DEFAULT_INSTANCE();

enum Mode {
  MODE_PLAY,
  MODE_CHANNEL_SELECT,
  MODE_SENS_SELECT,
  MODE_CALIBRATION
};

Mode currentMode = MODE_PLAY;

const int buttonPin = 2;
const int xPin = A0;
const int yPin = A1;
int midiChannel = 1;
int sensivity = 10;

bool buttonPressed = false;
bool longPress = false;
elapsedMillis buttonTimer;
const unsigned long longPressTime = 600;

int rawCenterX = 512;
int rawCenterY = 512;

int calibrationMinX = 1023;
int calibrationMaxX = 0;
int calibrationMinY = 1023;
int calibrationMaxY = 0;

void saveSettings() {
  EEPROM.update(0, midiChannel);
  EEPROM.update(1, sensivity);
  EEPROM.put(2, rawCenterX);
  EEPROM.put(4, rawCenterY);
}

void loadSettings() {
  midiChannel = EEPROM.read(0);
  sensivity = EEPROM.read(1);
  EEPROM.get(2, rawCenterX);
  EEPROM.get(4, rawCenterY);
  if (midiChannel < 1 || midiChannel > 16) midiChannel = 1;
  if (sensivity < 1 || sensivity > 20) sensivity = 10;
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  loadSettings();
}

void handleButton() {
  if (!digitalRead(buttonPin)) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonTimer = 0;
    } else {
      if (buttonTimer > longPressTime && !longPress) {
        longPress = true;
      }
    }
  } else {
    if (buttonPressed) {
      if (longPress) {
        switch (currentMode) {
          case MODE_PLAY:
            currentMode = MODE_CALIBRATION;
            break;
          case MODE_CALIBRATION:
            saveSettings();
            currentMode = MODE_PLAY;
            break;
          default:
            break;
        }
      } else {
        switch (currentMode) {
          case MODE_PLAY:
            currentMode = MODE_CHANNEL_SELECT;
            break;
          case MODE_CHANNEL_SELECT:
            midiChannel++;
            if (midiChannel > 16) midiChannel = 1;
            saveSettings();
            break;
          case MODE_SENS_SELECT:
            sensivity++;
            if (sensivity > 20) sensivity = 1;
            saveSettings();
            break;
          default:
            break;
        }
      }
    }
    buttonPressed = false;
    longPress = false;
  }
}

void handleJoystick() {
  int x = analogRead(xPin);
  int y = analogRead(yPin);
  x = map(x, calibrationMinX, calibrationMaxX, 0, 127);
  y = map(y, calibrationMinY, calibrationMaxY, 0, 127);
  x = constrain(x, 0, 127);
  y = constrain(y, 0, 127);
  MIDI.sendControlChange(1, x, midiChannel);
  MIDI.sendControlChange(2, y, midiChannel);
}

void handleCalibration() {
  int x = analogRead(xPin);
  int y = analogRead(yPin);
  if (x < calibrationMinX) calibrationMinX = x;
  if (x > calibrationMaxX) calibrationMaxX = x;
  if (y < calibrationMinY) calibrationMinY = y;
  if (y > calibrationMaxY) calibrationMaxY = y;
}

void loop() {
  handleButton();
  switch (currentMode) {
    case MODE_PLAY:
      handleJoystick();
      break;
    case MODE_CHANNEL_SELECT:
      break;
    case MODE_SENS_SELECT:
      break;
    case MODE_CALIBRATION:
      handleCalibration();
      break;
  }
}
