#include <Keypad.h>

const byte ROWS = 3; // rows
const byte COLS = 3; // columns
int state = 1; // Initial position

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};

byte rowPins[ROWS] = {2, 3, 4}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 6, 7}; // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    switch (key) {
      case '1': // Move forward
        moveForward();
        break;
      case '3': // Move backward
        moveBackward();
        break;
      default:
        break;
    }
  }
}

void moveForward() {
  switch(state) {
    case 1:
      electronicControl(8);
      state = 2;
      break;
    case 2:
      electronicControl(9);
      state = 3;
      break;
    case 3:
      electronicControl(10);
      state = 1;
      break;
  }
}

void moveBackward() {
  switch(state) {
    case 2:
      electronicControl(9);
      state = 1;
      break;
    case 3:
      electronicControl(10);
      state = 2;
      break;
    case 1:
      electronicControl(8);
      state = 3;
      break;
  }
}

void electronicControl(int pin) {
  digitalWrite(pin, HIGH);
  delay(150);
  digitalWrite(pin, LOW);
}