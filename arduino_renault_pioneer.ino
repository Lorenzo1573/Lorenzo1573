#include <Keypad.h>
#include <Encoder.h>

// Define the button matrix keys
const byte ROWS = 3; // three rows
const byte COLS = 3; // three columns
char keys[ROWS][COLS] = {
  {'V+', 'V-', ' '},
  {'M', 'S_prev', 'S_next'},
  {' ', 'Mute', ' '}
};

// Define the pin connections for rows and columns
byte rowPins[ROWS] = {2, 3, 4}; // Row pins
byte colPins[COLS] = {5, 6, 7}; // Column pins

// Create the keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Encoder pins
Encoder myEnc(8, 10);

// LED and output pins
const int ledPin = 13;
const int modePin = 12;
const int pwmPin = 9;
const int outputPin1 = 14;
const int outputPin2 = 15;
const int outputPin3 = 16;

// State variables
bool modeState = false;
long position  = 0;

void setup() {
  // Start serial communication
  Serial.begin(9600);

  // Setup pins
  pinMode(ledPin, OUTPUT);
  pinMode(modePin, INPUT_PULLUP);
  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin2, OUTPUT);
  pinMode(outputPin3, OUTPUT);

  // Initialize PWM pin
  pinMode(pwmPin, OUTPUT);
}

void loop() {
  // Read the keypad
  char key = keypad.getKey();
  if (key) {
    processKey(key);
  }

  // Read the encoder
  long newPosition = myEnc.read();
  if (newPosition != position) {
    position = newPosition;
    if (modeState) {
      // Handle encoder in Mode ON
    }
  }

  // Check mode button
  if (digitalRead(modePin) == LOW) {
    modeState = !modeState;
    digitalWrite(ledPin, modeState ? HIGH : LOW);
    delay(100);
  }
}

void processKey(char key) {
  switch (key) {
    case 'V+':
      sendCommand(16.8);
      break;
    case 'V-':
      sendCommand(23.6);
      break;
    // Other cases for Mute, Source etc.
  }
}

void sendCommand(float value) {
  if (modeState) {
    digitalWrite(outputPin1, LOW);
    delay(500);
    digitalWrite(outputPin1, HIGH);
  } else {
    analogWrite(pwmPin, map(value, 0, 100, 0, 255));
  }
}
