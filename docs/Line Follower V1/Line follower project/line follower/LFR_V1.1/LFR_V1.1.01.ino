// ============================================================================
// Project : Line Follower Robot (Lane-Keeping Between Two Guide Lines)
// Version : LFR_V1.1.01 (Iteration of V1.0 — IR Sensor + Motor Response Test)
// Board   : Arduino Uno R3
// Driver  : TB6612FNG Dual DC Motor Driver
// Sensors : 2x Hobbyist IR Sensor Modules (Front-Left & Front-Right)
// ============================================================================
// Behavior (v1.1.01 Obvious Test Mode):
//   - Robot stays BETWEEN two boundary lines (like driving inside a road lane).
//   - Neither sensor on line -> Both wheels spin forward at baseSpeed.
//   - Left sensor detects line -> Left wheel spins FASTER (fastSpeed),
//                                 Right wheel STOPS (0) [steers right, back into lane].
//   - Right sensor detects line -> Right wheel spins FASTER (fastSpeed),
//                                  Left wheel STOPS (0) [steers left, back into lane].
//   - Both sensors detect line -> Both motors STOP.
// ============================================================================

// ===== PIN DEFINITIONS =====

// Motor A (Left) — TB6612FNG Channel A
#define AIN1 8
#define AIN2 9
#define PWMA 5

// Motor B (Right) — TB6612FNG Channel B
#define BIN1 10
#define BIN2 11
#define PWMB 6

// TB6612FNG Standby pin (must be HIGH to enable driver)
#define STBY 7

// Front IR Sensors (Digital Output 'DO' / 'OUT' pins)
#define IR_LEFT  2   // Front-Left IR sensor DO pin
#define IR_RIGHT 3   // Front-Right IR sensor DO pin

// ===== SENSOR LOGIC CONFIGURATION =====
// Standard LM393 hobbyist IR modules output:
//   - HIGH (1) on BLACK line (IR absorbed, no reflection)
//   - LOW  (0) on WHITE surface (IR reflected)
// Flip this to LOW if testing with white boundary tape on a dark floor.
#define LINE_DETECTED HIGH

// ===== SPEED SETTINGS (0–255) =====
// Exaggerated speeds for v1.1.01 bench/floor verification (no tuning needed yet)
int baseSpeed = 140; // Normal cruising speed when centered between lines
int fastSpeed = 220; // Boosted speed on the wheel whose side detected the line
int stopSpeed = 0;   // Other wheel stops completely for obvious visual feedback

// ===== SETUP =====
void setup() {
  // Motor driver pins
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);

  // IR sensor input pins
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);

  // Enable TB6612FNG motor driver
  digitalWrite(STBY, HIGH);

  // Serial monitor for live sensor & motor state verification
  Serial.begin(9600);
  Serial.println("LFR V1.1.01 Initialized - Dual Boundary Line Test");
}

// ===== MAIN LOOP =====
void loop() {
  bool leftDetected  = (digitalRead(IR_LEFT)  == LINE_DETECTED);
  bool rightDetected = (digitalRead(IR_RIGHT) == LINE_DETECTED);

  if (!leftDetected && !rightDetected) {
    // Clear road between both guide lines -> Both wheels forward at baseSpeed
    driveForward(baseSpeed, baseSpeed);
    Serial.println("STATUS: CLEAR      | L_IR: 0 | R_IR: 0 | L_PWM: 140 | R_PWM: 140");
  }
  else if (leftDetected && !rightDetected) {
    // Left boundary line detected -> Left wheel FASTER, Right wheel STOPS (steers right)
    driveForward(fastSpeed, stopSpeed);
    Serial.println("STATUS: LEFT LINE  | L_IR: 1 | R_IR: 0 | L_PWM: 220 | R_PWM: 0 (STOP)");
  }
  else if (!leftDetected && rightDetected) {
    // Right boundary line detected -> Right wheel FASTER, Left wheel STOPS (steers left)
    driveForward(stopSpeed, fastSpeed);
    Serial.println("STATUS: RIGHT LINE | L_IR: 0 | R_IR: 1 | L_PWM: 0 (STOP) | R_PWM: 220");
  }
  else {
    // Both sensors hit a line -> Full stop
    stopMotors();
    Serial.println("STATUS: BOTH LINES | L_IR: 1 | R_IR: 1 | MOTORS STOPPED");
  }

  delay(20); // Short stability delay
}

// ===== MOTOR FUNCTIONS =====

// Drive both motors forward with independent Left and Right PWM speeds (0–255)
void driveForward(int leftPWM, int rightPWM) {
  // Left Motor (Motor A): Forward if leftPWM > 0, else Coast/Stop
  if (leftPWM > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  }

  // Right Motor (Motor B): Forward if rightPWM > 0, else Coast/Stop
  if (rightPWM > 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  }

  analogWrite(PWMA, leftPWM);
  analogWrite(PWMB, rightPWM);
}

// Legacy V1.0 helper functions (preserved for reference/testing)
void forward() {
  driveForward(baseSpeed, baseSpeed);
}

void backward() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, baseSpeed);
  analogWrite(PWMB, baseSpeed);
}

void turnLeft() {
  // Pivot left in place (Left backward, Right forward)
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, baseSpeed);
  analogWrite(PWMB, baseSpeed);
}

void turnRight() {
  // Pivot right in place (Left forward, Right backward)
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, baseSpeed);
  analogWrite(PWMB, baseSpeed);
}

// Stop both motors (coast)
void stopMotors() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}
