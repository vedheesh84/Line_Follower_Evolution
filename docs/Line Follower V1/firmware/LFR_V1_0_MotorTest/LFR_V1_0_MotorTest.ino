// ============================================================================
// Project : Line Follower Robot (V1 Hardware Proving Ground)
// Version : LFR_V1.0_MotorTest (Open-Loop TB6612FNG Motor Driver Verification)
// Board   : Arduino Uno R3 (ATmega328P)
// Driver  : TB6612FNG Dual DC Motor Driver
// ============================================================================
// Verification Objective:
//   - Verify Arduino digital & PWM signal generation through TB6612FNG driver.
//   - Validate motor direction switching (AIN1/AIN2 and BIN1/BIN2) and speed control (PWMA/PWMB).
//   - Confirm driver STBY logic behavior (must be HIGH to exit standby mode).
// ============================================================================

// ===== PIN DEFINITIONS =====

// Motor A (Left)
#define AIN1 8
#define AIN2 9
#define PWMA 5

// Motor B (Right)
#define BIN1 10
#define BIN2 11
#define PWMB 6

// Standby pin
#define STBY 7

int speedVal = 150; // speed (0–255)

// ===== SETUP =====
void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH); // enable motor driver
}

// ===== MAIN LOOP =====
void loop() {
  forward();
  delay(2000);

  backward();
  delay(2000);

  turnLeft();
  delay(2000);

  turnRight();
  delay(2000);

  stopMotors();
  delay(2000);
}

// ===== MOTOR FUNCTIONS =====

// Move forward
void forward() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
}

// Move backward
void backward() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
}

// Turn left
void turnLeft() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);  // left motor backward

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);   // right motor forward

  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
}

// Turn right
void turnRight() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);   // left motor forward

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);  // right motor backward

  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
}

// Stop motors (coast)
void stopMotors() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}
