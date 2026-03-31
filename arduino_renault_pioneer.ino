// ============================================================
//  Arduino Renault → Pioneer steering wheel remote adapter
//
//  Hardware
//  --------
//  Keypad 3×3:  rows → A0 (r1), A1 (r2), A2 (r3)
//               cols → A3 (c1), A4 (c2), A5 (c3)
//
//  Key layout:
//    [1]=enc-A  [2]=enc-B  [3]=enc-C
//    [4]=Mute   [5]=Src+   [6]=Src-
//    [7]=Vol+   [8]=Mode   [9]=Vol-
//
//  Output pins (Pioneer resistive bus)
//  ------------------------------------
//  All control outputs sit idle-HIGH.
//  A command is sent by driving the line LOW for PULSE_DURATION ms,
//  then returning it HIGH.  This way the Pioneer always reads open
//  (no resistor) when idle and reads only the desired resistor when
//  a button is pressed.
//
//  D2  – Encoder Forward  (default, active when MODE off)
//  D3  – Encoder Backward (default, active when MODE off)
//  D4  – MODE bistable output (LOW=mode off, HIGH=mode on)
//  D5  – Encoder Forward  (alt,     active when MODE on)
//  D6  – Encoder Backward (alt,     active when MODE on)
//  D7  – Mute             (alt,     active when MODE on)
//  D8  – Mute             (default, active when MODE off)
//  D9  – Source +
//  D10 – Source -
//  D11 – Volume +
//  D13 – Volume -
//
//  Rotary encoder quadrature (3-position Gray-code wheel)
//  -------------------------------------------------------
//  Positions: A=1, B=2, C=3
//  Forward  sequence: A→B→C→A
//  Backward sequence: A→C→B→A
//  First keypad read initialises the position; no command is sent.
// ============================================================

#include <Keypad.h>

// ---- Keypad ------------------------------------------------
static const byte ROWS = 3;
static const byte COLS = 3;

static char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};

static byte rowPins[ROWS] = {A0, A1, A2};
static byte colPins[COLS] = {A3, A4, A5};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---- Output pin definitions --------------------------------
#define PIN_ENC_FWD       2   // Encoder Forward  (default)
#define PIN_ENC_BWD       3   // Encoder Backward (default)
#define PIN_MODE          4   // Mode bistable (LOW=off, HIGH=on)
#define PIN_ENC_FWD_ALT   5   // Encoder Forward  (alt, when MODE on)
#define PIN_ENC_BWD_ALT   6   // Encoder Backward (alt, when MODE on)
#define PIN_MUTE_ALT      7   // Mute             (alt, when MODE on)
#define PIN_MUTE          8   // Mute             (default)
#define PIN_SRC_PLUS      9   // Source +
#define PIN_SRC_MINUS    10   // Source -
#define PIN_VOL_PLUS     11   // Volume +
#define PIN_VOL_MINUS    13   // Volume -

// ---- Timing ------------------------------------------------
#define PULSE_DURATION  150   // ms – Pioneer command pulse width

// ---- Runtime state -----------------------------------------
static bool modeOn     = false;  // Current MODE bistable state
static int  encoderPos = 0;      // 0=uninitialised, 1=A, 2=B, 3=C

// ============================================================
//  Helper: drive a Pioneer control line LOW for PULSE_DURATION
//  then return it HIGH.  When MODE is on, encoder/mute commands
//  are automatically rerouted to the alt pins.
// ============================================================
static void sendCommand(int pin) {
  int p = pin;
  if (modeOn) {
    if      (pin == PIN_ENC_FWD) p = PIN_ENC_FWD_ALT;
    else if (pin == PIN_ENC_BWD) p = PIN_ENC_BWD_ALT;
    else if (pin == PIN_MUTE)    p = PIN_MUTE_ALT;
  }
  digitalWrite(p, LOW);
  delay(PULSE_DURATION);
  digitalWrite(p, HIGH);
}

// ============================================================
//  Mode bistable toggle
//  MODE off → on : D2/D3/D8 become hi-Z, D5/D6/D7 become outputs
//  MODE on  → off: D5/D6/D7 become hi-Z, D2/D3/D8 become outputs
// ============================================================
static void activateMode() {
  // Release default encoder/mute pins (hi-Z = not connected to Pioneer bus)
  pinMode(PIN_ENC_FWD, INPUT);
  pinMode(PIN_ENC_BWD, INPUT);
  pinMode(PIN_MUTE,    INPUT);

  // Enable alt encoder/mute pins, idle HIGH
  pinMode(PIN_ENC_FWD_ALT, OUTPUT); digitalWrite(PIN_ENC_FWD_ALT, HIGH);
  pinMode(PIN_ENC_BWD_ALT, OUTPUT); digitalWrite(PIN_ENC_BWD_ALT, HIGH);
  pinMode(PIN_MUTE_ALT,    OUTPUT); digitalWrite(PIN_MUTE_ALT,    HIGH);

  modeOn = true;
  digitalWrite(PIN_MODE, HIGH);
  Serial.println("Mode ON  – D5/D6/D7 attivi");
}

static void deactivateMode() {
  // Release alt encoder/mute pins
  pinMode(PIN_ENC_FWD_ALT, INPUT);
  pinMode(PIN_ENC_BWD_ALT, INPUT);
  pinMode(PIN_MUTE_ALT,    INPUT);

  // Re-enable default encoder/mute pins, idle HIGH
  pinMode(PIN_ENC_FWD, OUTPUT); digitalWrite(PIN_ENC_FWD, HIGH);
  pinMode(PIN_ENC_BWD, OUTPUT); digitalWrite(PIN_ENC_BWD, HIGH);
  pinMode(PIN_MUTE,    OUTPUT); digitalWrite(PIN_MUTE,    HIGH);

  modeOn = false;
  digitalWrite(PIN_MODE, LOW);
  Serial.println("Mode OFF – D2/D3/D8 attivi");
}

// ============================================================
//  Encoder direction detection
//  nextForward(pos)  → position that follows pos going forward
//  nextBackward(pos) → position that follows pos going backward
// ============================================================
static int nextForward(int pos) {
  // A→B→C→A  (1→2→3→1)
  return (pos % 3) + 1;
}

static int nextBackward(int pos) {
  // A→C→B→A  (1→3→2→1)
  if (pos == 1) return 3;
  if (pos == 3) return 2;
  return 1; // pos == 2
}

static void handleEncoder(int newPos) {
  if (encoderPos == 0) {
    // First read: initialise position, send no command
    encoderPos = newPos;
    Serial.print("Encoder inizializzato – pos ");
    Serial.println(newPos);
    return;
  }

  if (newPos == nextForward(encoderPos)) {
    Serial.print("Encoder Avanti: ");
    Serial.print(encoderPos); Serial.print("→"); Serial.println(newPos);
    sendCommand(PIN_ENC_FWD);
  } else if (newPos == nextBackward(encoderPos)) {
    Serial.print("Encoder Indietro: ");
    Serial.print(encoderPos); Serial.print("→"); Serial.println(newPos);
    sendCommand(PIN_ENC_BWD);
  }
  // else: same position re-read – ignore
  encoderPos = newPos;
}

// ============================================================
//  setup()
// ============================================================
void setup() {
  Serial.begin(9600);

  // Default encoder/mute outputs – idle HIGH
  pinMode(PIN_ENC_FWD, OUTPUT); digitalWrite(PIN_ENC_FWD, HIGH);
  pinMode(PIN_ENC_BWD, OUTPUT); digitalWrite(PIN_ENC_BWD, HIGH);
  pinMode(PIN_MUTE,    OUTPUT); digitalWrite(PIN_MUTE,    HIGH);

  // Alt pins start as hi-Z (unused until MODE is activated)
  pinMode(PIN_ENC_FWD_ALT, INPUT);
  pinMode(PIN_ENC_BWD_ALT, INPUT);
  pinMode(PIN_MUTE_ALT,    INPUT);

  // MODE pin: starts LOW (mode off)
  pinMode(PIN_MODE, OUTPUT);
  digitalWrite(PIN_MODE, LOW);

  // Remaining button outputs – idle HIGH
  pinMode(PIN_SRC_PLUS,  OUTPUT); digitalWrite(PIN_SRC_PLUS,  HIGH);
  pinMode(PIN_SRC_MINUS, OUTPUT); digitalWrite(PIN_SRC_MINUS, HIGH);
  pinMode(PIN_VOL_PLUS,  OUTPUT); digitalWrite(PIN_VOL_PLUS,  HIGH);
  pinMode(PIN_VOL_MINUS, OUTPUT); digitalWrite(PIN_VOL_MINUS, HIGH);

  Serial.println("Sistema avviato – Mode OFF");
}

// ============================================================
//  loop()
// ============================================================
void loop() {
  char key = keypad.getKey();
  if (key == NO_KEY) return;

  Serial.print("Tasto: ");
  Serial.println(key);

  switch (key) {
    // Encoder positions
    case '1': handleEncoder(1);                                        break;
    case '2': handleEncoder(2);                                        break;
    case '3': handleEncoder(3);                                        break;

    // Direct buttons
    case '4': sendCommand(PIN_MUTE);      Serial.println("Mute");     break;
    case '5': sendCommand(PIN_SRC_PLUS);  Serial.println("Source +"); break;
    case '6': sendCommand(PIN_SRC_MINUS); Serial.println("Source -"); break;
    case '7': sendCommand(PIN_VOL_PLUS);  Serial.println("Vol +");    break;
    case '9': sendCommand(PIN_VOL_MINUS); Serial.println("Vol -");    break;

    // Mode bistable toggle
    case '8':
      if (modeOn) deactivateMode();
      else        activateMode();
      break;
  }
}
