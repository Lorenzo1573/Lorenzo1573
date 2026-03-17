#include <Keypad.h>

const byte ROWS = 3;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};

byte rowPins[ROWS] = {A0, A1, A2}; // Row pins: A0=row1, A1=row2, A2=row3
byte colPins[COLS] = {A3, A4, A5}; // Col pins: A3=col1, A4=col2, A5=col3

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Encoder output pins (D2/D3 are free since matrix moved to A0-A5)
#define PIN_ENCODER_FWD  2  // Encoder Forward
#define PIN_ENCODER_BWD  3  // Encoder Backward

// Button output pins
#define PIN_MUTE         8  // Mute       - Tasto 4 (A1+A3)
#define PIN_SOURCE_PLUS  9  // Source +   - Tasto 5 (A1+A4)
#define PIN_SOURCE_MINUS 10 // Source -   - Tasto 6 (A1+A5)
#define PIN_VOL_PLUS     11 // Vol +      - Tasto 7 (A2+A3)
#define PIN_MODE         12 // Mode ON/OFF - Tasto 8 (A2+A4)
#define PIN_VOL_MINUS    13 // Vol -      - Tasto 9 (A2+A5)

// Encoder position tracking
// 0 = not yet initialized
// 1 = Position A (A0+A3, key '1')
// 2 = Position B (A0+A4, key '2')
// 3 = Position C (A0+A5, key '3')
// Forward sequence:  A->B->C->A  (1->2->3->1)
// Backward sequence: A->C->B->A  (1->3->2->1)
int encoderPos = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_ENCODER_FWD, OUTPUT);
  pinMode(PIN_ENCODER_BWD, OUTPUT);
  pinMode(PIN_MUTE, OUTPUT);
  pinMode(PIN_SOURCE_PLUS, OUTPUT);
  pinMode(PIN_SOURCE_MINUS, OUTPUT);
  pinMode(PIN_VOL_PLUS, OUTPUT);
  pinMode(PIN_MODE, OUTPUT);
  pinMode(PIN_VOL_MINUS, OUTPUT);
  Serial.println("Sistema avviato - Mode OFF");
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    Serial.print("Tasto: ");
    Serial.println(key);

    switch (key) {
      case '1': handleEncoder(1); break; // Encoder pos A (A0+A3)
      case '2': handleEncoder(2); break; // Encoder pos B (A0+A4)
      case '3': handleEncoder(3); break; // Encoder pos C (A0+A5)
      case '4': electronicControl(PIN_MUTE);         Serial.println("Mute");      break;
      case '5': electronicControl(PIN_SOURCE_PLUS);  Serial.println("Source +");  break;
      case '6': electronicControl(PIN_SOURCE_MINUS); Serial.println("Source -");  break;
      case '7': electronicControl(PIN_VOL_PLUS);     Serial.println("Vol +");     break;
      case '8': electronicControl(PIN_MODE);         Serial.println("Mode");      break;
      case '9': electronicControl(PIN_VOL_MINUS);    Serial.println("Vol -");     break;
    }
  }
}

// Returns the next position when rotating forward from pos
// Forward: A(1)->B(2)->C(3)->A(1)
int nextForward(int pos) {
  if (pos == 1) return 2;
  if (pos == 2) return 3;
  return 1; // pos == 3
}

// Returns the next position when rotating backward from pos
// Backward: A(1)->C(3)->B(2)->A(1)
int nextBackward(int pos) {
  if (pos == 1) return 3;
  if (pos == 3) return 2;
  return 1; // pos == 2
}

void handleEncoder(int newPos) {
  if (encoderPos == 0) {
    // First movement: initialize position, no command sent
    encoderPos = newPos;
    Serial.print("Encoder inizializzato - Posizione: ");
    Serial.println(newPos);
  } else if (newPos == nextForward(encoderPos)) {
    // Forward movement detected
    Serial.print("Encoder Avanti: ");
    Serial.print(encoderPos);
    Serial.print(" -> ");
    Serial.println(newPos);
    electronicControl(PIN_ENCODER_FWD);
    encoderPos = newPos;
  } else if (newPos == nextBackward(encoderPos)) {
    // Backward movement detected
    Serial.print("Encoder Indietro: ");
    Serial.print(encoderPos);
    Serial.print(" -> ");
    Serial.println(newPos);
    electronicControl(PIN_ENCODER_BWD);
    encoderPos = newPos;
  } else {
    // Same position re-read or unexpected: just update
    encoderPos = newPos;
  }
}

void electronicControl(int pin) {
  digitalWrite(pin, HIGH);
  delay(150);
  digitalWrite(pin, LOW);
}