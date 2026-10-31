// ============================================================================
// Project : Line Follower Robot (Lane-Keeping Between Two Guide Lines)
// Version : LFR_V1.1.02 (Low-Inrush / Soft-Start Edition for 12V 1.5A Adapter)
// Board   : Arduino Uno R3
// Driver  : TB6612FNG Dual DC Motor Driver
// Sensors : 2x Hobbyist IR Sensor Modules (Front-Left & Front-Right)
// ============================================================================
// Why Soft-Start & Lower PWM Cap Are Included:
//   - On a 12V 1.5A (~1.2A actual) wall adapter, starting both motors abruptly
//     at high PWM causes a 1.6A-2.5A inrush current spike that trips the
//     adapter's over-current protection (voltage collapse).
//   - This code uses:
//       1) Staggered Soft-Start Ramping (ramping PWM smoothly & staggering
//          Motor A and Motor B startups so inrush spikes never overlap).
//       2) 12V-Safe PWM Speeds (baseSpeed = 85 [~4.0V], fastSpeed = 130 [~6.1V]).
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

// ===== 12V-SAFE SPEED & SOFT-START SETTINGS (0–255) =====
// With VM = 12V:
//   PWM 85  ~= 4.0V effective motor voltage
//   PWM 130 ~= 6.1V effective motor voltage
int baseSpeed = 85;  // Normal cruising speed when centered between lines
int fastSpeed = 130; // Boosted speed on the wheel whose side detected the line
int stopSpeed = 0;   // Other wheel stops completely for obvious visual feedback

// Soft-start ramp step (limits peak current draw from 12V adapter)
const int RAMP_STEP = 15; // Max PWM increase per loop iteration (~15ms)

// Track current motor PWM to ramp smoothly instead of spiking
int currentLeftPWM  = 0;
int currentRightPWM = 0;

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

  // Ensure motors start at 0 before enabling driver
  stopMotorsImmediate();
  digitalWrite(STBY, HIGH);

  // Serial monitor for live sensor & motor state verification
  Serial.begin(9600);
  Serial.println("LFR V1.1.02 Initialized (Soft-Start Mode for 12V Adapter)");

  // Staggered initial spin-up delay so power rails stabilize first
  delay(500);
}

// ===== MAIN LOOP =====
void loop() {
  bool leftDetected  = (digitalRead(IR_LEFT)  == LINE_DETECTED);
  bool rightDetected = (digitalRead(IR_RIGHT) == LINE_DETECTED);

  if (!leftDetected && !rightDetected) {
    // Clear road between both guide lines -> Both wheels forward at baseSpeed
    driveForwardSoft(baseSpeed, baseSpeed);
    Serial.print("CLEAR      | L_PWM: ");
    Serial.print(currentLeftPWM);
    Serial.print(" | R_PWM: ");
    Serial.println(currentRightPWM);
  }
  else if (leftDetected && !rightDetected) {
    // Left boundary line detected -> Left wheel FASTER, Right wheel STOPS (steers right)
    driveForwardSoft(fastSpeed, stopSpeed);
    Serial.print("LEFT LINE  | L_PWM: ");
    Serial.print(currentLeftPWM);
    Serial.print(" | R_PWM: ");
    Serial.println(currentRightPWM);
  }
  else if (!leftDetected && rightDetected) {
    // Right boundary line detected -> Right wheel FASTER, Left wheel STOPS (steers left)
    driveForwardSoft(stopSpeed, fastSpeed);
    Serial.print("RIGHT LINE | L_PWM: ");
    Serial.print(currentLeftPWM);
    Serial.print(" | R_PWM: ");
    Serial.println(currentRightPWM);
  }
  else {
    // Both sensors hit a line -> Full stop
    stopMotorsImmediate();
    Serial.println("BOTH LINES | MOTORS STOPPED");
  }

  delay(15); // ~66 Hz soft-ramp update rate
}

// ===== SOFT-START MOTOR FUNCTIONS =====

// Smoothly ramps Left and Right motors toward target PWM and staggers startup
// from 0 so both motors never draw locked-rotor stall current at the same instant.
void driveForwardSoft(int targetLeftPWM, int targetRightPWM) {
  // Staggered startup: if BOTH motors are currently stopped (0) and both want
  // to start, start Left Motor first and hold Right Motor at 0 until Left reaches ~45 PWM.
  if (currentRightPWM == 0 && targetRightPWM > 0 && currentLeftPWM < 45 && targetLeftPWM > 0) {
    targetRightPWM = 0; // Wait ~45ms for Left motor inrush spike to pass
  }

  // Ramp Left Motor PWM smoothly
  if (currentLeftPWM < targetLeftPWM) {
    currentLeftPWM = min(currentLeftPWM + RAMP_STEP, targetLeftPWM);
  } else if (currentLeftPWM > targetLeftPWM) {
    // Stopping/slowing down can happen immediately (costs 0 current)
    currentLeftPWM = targetLeftPWM;
  }

  // Ramp Right Motor PWM smoothly
  if (currentRightPWM < targetRightPWM) {
    currentRightPWM = min(currentRightPWM + RAMP_STEP, targetRightPWM);
  } else if (currentRightPWM > targetRightPWM) {
    currentRightPWM = targetRightPWM;
  }

  // Apply Left Motor (Channel A)
  if (currentLeftPWM > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  }
  analogWrite(PWMA, currentLeftPWM);

  // Apply Right Motor (Channel B)
  if (currentRightPWM > 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  }
  analogWrite(PWMB, currentRightPWM);
}

// Immediate coast stop (resets soft-start counters to 0)
void stopMotorsImmediate() {
  currentLeftPWM  = 0;
  currentRightPWM = 0;

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}
