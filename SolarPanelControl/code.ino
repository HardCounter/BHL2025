#include <Servo.h>

// --- Configuration ---
const int servoPin = 9;
const int leftSensorPin = A4;
const int rightSensorPin = A5;

// --- Settings ---
const int servoMin = 10;
const int servoMax = 170;
const int stepDelay = 50;     // Give servo 50ms to physically move 1 degree
const int tolerance = 40;     // Sensitivity
const int wakeThreshold = 50; 

// --- Variables ---
Servo solarServo;
int currentAngle = 90;
int parkedTotalBrightness = 0;
bool isSleeping = true;

// Stability Timer: How long must we be aligned before sleeping?
int alignedCounter = 0;       
const int alignedThreshold = 20; // Needs ~1 second (20 * 50ms) of stable light to sleep

void setup() {
  Serial.begin(9600);
  solarServo.attach(servoPin);
  pinMode(leftSensorPin, INPUT);
  pinMode(rightSensorPin, INPUT);

  Serial.println("--- Init ---");
  
  // Set initial position
  solarServo.write(currentAngle);
  delay(500);
  
  // Take initial reading for sleep baseline
  parkedTotalBrightness = analogRead(leftSensorPin) + analogRead(rightSensorPin);
}

void loop() {
  // 1. Read Sensors
  int leftVal = analogRead(leftSensorPin) - 100; // Your calibration
  int rightVal = analogRead(rightSensorPin);
  int currentTotal = leftVal + rightVal;
  int diff = leftVal - rightVal;

  // --- LOGIC ---

  if (isSleeping) {
    // --- SLEEP MODE ---
    // Check if light changed significantly
    if (abs(currentTotal - parkedTotalBrightness) > wakeThreshold) {
      Serial.println("Waking up...");
      isSleeping = false;
      alignedCounter = 0; // Reset stability counter
    }
    // Sleep Check Rate (Slow)
    delay(1000); 

  } else {
    // --- ACTIVE TRACKING (Step-by-Step) ---

    // Are we aligned?
    if (abs(diff) <= tolerance) {
      // Yes, we are aligned. But don't sleep yet!
      // Wait to be sure it's stable.
      alignedCounter++;
      
      if (alignedCounter > alignedThreshold) {
        Serial.println("Stable and Aligned. Sleeping.");
        parkedTotalBrightness = currentTotal;
        isSleeping = true;
      }
    } 
    else {
      // No, we are NOT aligned. Move ONE step only.
      alignedCounter = 0; // Reset stability because we are moving

      if (leftVal > rightVal) {
        currentAngle--; 
      } else {
        currentAngle++;
      }

      // Safety Limits
      currentAngle = constrain(currentAngle, servoMin, servoMax);

      // Mechanical Limit Check
      if (currentAngle == servoMin || currentAngle == servoMax) {
        Serial.println("Hit mechanical limit. Sleeping.");
        parkedTotalBrightness = currentTotal;
        isSleeping = true;
      }

      // MOVE THE SERVO
      solarServo.write(currentAngle);
      
      // CRITICAL: Wait for the servo to physically reach the new spot
      // Before we loop again to read sensors.
      delay(stepDelay); 
    }
    
    // Slight delay for loop stability
    delay(10);
  }
}
