// Arduino code for interfacing Renault Megane 2004 steering wheel controls
// with Pioneer Avic HD1BT using a 3x3 matrix and PWM resistor simulation.

#include <Encoder.h>

// Define matrix size
const int rows = 3; 
const int cols = 3;
const int buttonPins[rows][cols] = {
    {2, 3, 4},
    {5, 6, 7},
    {8, 9, 10}
};

Encoder trackEncoder(11, 12); // Encoder pins
int modeButtonPin = 13; // Mode button pin
int ledPin = LED_BUILTIN; // LED pin
int pwmValue = 0;

void setup() {
    // Set up button pins as input
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            pinMode(buttonPins[i][j], INPUT_PULLUP);
        }
    }
    pinMode(modeButtonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    Serial.begin(9600); // Initialize serial communication
}

void loop() {
    readButtons();
    trackControl();
    delay(100); // Small delay for debouncing
}

void readButtons() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (digitalRead(buttonPins[i][j]) == LOW) { // Button pressed
                Serial.print("Button pressed: ");
                Serial.println(i * cols + j);
            }
        }
    }
    
    if (digitalRead(modeButtonPin) == LOW) {
        digitalWrite(ledPin, !digitalRead(ledPin)); // Toggle LED
        Serial.println("Mode button pressed");
    }
}

void trackControl() {
    long pos = trackEncoder.read(); // Read encoder position
    Serial.print("Track Position: ");
    Serial.println(pos);
    
    // Simulate PWM control using position
    pwmValue = map(pos, 0, 100, 0, 255); // Example mapping
    analogWrite(ledPin, pwmValue); // Example output to LED
}