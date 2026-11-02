# Line Follower Robot (V1 Physical Embodiment)

**Document Type:** Project Primary Documentation (Standard Project Pattern)  
**Scope:** `03 — Projects & Experiments/Active/Line Follower Robot`  
**Physical Directory:** `E:\Line follower project`  
**Status:** Active (Awaiting SMPS Power Supply Delivery)  
**Hardware Target:** Arduino Uno R3 (ATmega328P) + TB6612FNG Motor Driver + 2x IR Sensors + 2WD Differential Drive Chassis  
**Primary Domain:** [ROS2](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/02%20%E2%80%94%20Domains/ROS2/README.md) / Robotics  
**Theoretical Reference:** [Article LFE-02: V1 Reactive Physics & Real-World Instability](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/02%20%E2%80%94%20Domains/ROS2/ROS2%20Articles/07_Line_Follower_Evolution_Series/article_lfe_02_v1_reactive_physics_and_instability.md)  
**Associated Simulation Workspace:** [Line_Follower_Evolution_ws](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/02%20%E2%80%94%20Domains/ROS2/Line_Follower_Evolution_ws/README.md)  

---

## 1. Overview

The **Line Follower Robot (Physical V1)** is the physical hardware proving ground for the first evolutionary generation within the Intelligent Systems Engineering Workspace.

While the accompanying [Line_Follower_Evolution_ws](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/02%20%E2%80%94%20Domains/ROS2/Line_Follower_Evolution_ws/README.md) models this robot in ROS 2 simulation, this physical build confronts the real-world constraints of discrete hardware: logic voltage switching, back-EMF, motor inrush current spikes, power supply brownouts, sensor thresholding, and bang-bang steering dynamics.

---

## 2. Intent

> **"Build the physical baseline from first principles to expose and understand the real dynamics that pure software abstractions hide."**

The purpose of V1 is to:
1. Establish a working, self-contained 2-sensor differential drive robot on physical hardware.
2. Verify hardware abstraction from raw gate switching up to microcontroller software loops.
3. Observe and document the physical failure modes of reactive bang-bang thresholding (overshoot, limit-cycle oscillation, phase lag, and voltage drops) before attempting closed-loop PID or SLAM.
4. Establish clean bench testing and diagnostic practices (power split, star grounding, board health verification).

---

## 3. Context & Boundaries

- **Physical System Boundary:**
  - Compute: Single 8-bit Microcontroller (Arduino Uno R3, ATmega328P).
  - Actuation: TB6612FNG Dual MOSFET H-bridge motor driver driving 2 geared DC hobby motors.
  - Sensing: 2 discrete optical reflectance IR sensor modules (LM393 comparators) positioned at the front-left and front-right corners.
  - Power: DC Power Supply / SMPS (9V–12V DC) split into raw motor power (`VM`) and regulated logic power (`5V` via buck converter / onboard linear regulator).
- **Non-Goals for V1:**
  - No wheel encoders or closed-loop velocity PID (reserved for Generation 2: V2).
  - No continuous analog array centroid estimation (reserved for Generation 2: V3).
  - No camera, AprilTags, or topological graph navigation (reserved for Generation 3: V4–V5).
  - No LiDAR, SLAM, or ROS 2 onboard SBC compute (reserved for Generation 4: V6).

---

## 4. Structure & Organization

This project folder strictly adheres to the **Intelligent Systems Engineering Workspace Project Pattern**:

```text
E:\Line follower project\
├── README.md                                  # Primary Project Documentation (this file)
│
├── firmware/                                  # Arduino IDE-compliant sketch directories
│   ├── LFR_V1_0_MotorTest/
│   │   └── LFR_V1_0_MotorTest.ino             # Phase 1: Open-loop TB6612FNG motor driver test
│   ├── LFR_V1_1_01_DualBoundary/
│   │   └── LFR_V1_1_01_DualBoundary.ino       # Phase 2: Dual IR boundary lane-keeping logic
│   └── LFR_V1_1_02_SoftStart/
│       └── LFR_V1_1_02_SoftStart.ino          # Phase 2b: Soft-start PWM ramping for inrush mitigation
│
└── resources/                                 # Supporting technical evidence & specifications
    ├── WIRING_DIAGRAM_V1.1.01.md              # Canonical schematic, power split, and pin tables
    ├── arduino_diagnosis.md                   # Hardware verification report for Arduino Boards #1 & #2
    └── PROJECT_REPORT_HISTORICAL.md           # Retained migration and historical cleanup log
```

---

## 5. Behaviour & Operation

### 5.1. Pin Mapping Architecture

| Hardware Component | Component Pin | Arduino Uno Pin | Mode / Function |
| :--- | :--- | :--- | :--- |
| **TB6612FNG Motor A (Left)** | `PWMA` | `D5` (PWM) | Speed control (`analogWrite` 0–255) |
| | `AIN1` | `D8` | Direction control bit 1 |
| | `AIN2` | `D9` | Direction control bit 2 |
| **TB6612FNG Motor B (Right)**| `PWMB` | `D6` (PWM) | Speed control (`analogWrite` 0–255) |
| | `BIN1` | `D10` | Direction control bit 1 |
| | `BIN2` | `D11` | Direction control bit 2 |
| **TB6612FNG Control** | `STBY` | `D7` | Standby enable (`HIGH` = Active, `LOW` = Coast/Off) |
| **Front-Left IR Sensor** | `DO` / `OUT` | `D2` | Digital boundary detect (`HIGH` = line, `LOW` = floor) |
| **Front-Right IR Sensor** | `DO` / `OUT` | `D3` | Digital boundary detect (`HIGH` = line, `LOW` = floor) |

### 5.2. Control Logic Table (Dual Boundary Lane Keeping)

The V1.1 firmware implements a lane-keeping threshold strategy between two guide lines:

| Left IR (`D2`) | Right IR (`D3`) | Tracking State | Left Motor (`PWMA`) | Right Motor (`PWMB`) | Physical Reaction |
| :---: | :---: | :---: | :---: | :---: | :--- |
| `0` (Floor) | `0` (Floor) | **CENTERED** | Forward (`baseSpeed`) | Forward (`baseSpeed`) | Robot cruises forward down center of lane |
| `1` (Line) | `0` (Floor) | **HIT LEFT** | Boost (`fastSpeed`) | Stop (`0`) | Differential yaw steer to the right |
| `0` (Floor) | `1` (Line) | **HIT RIGHT** | Stop (`0`) | Boost (`fastSpeed`) | Differential yaw steer to the left |
| `1` (Line) | `1` (Line) | **CROSSBAR / LOST** | Stop (`0`) | Stop (`0`) | Full emergency stop |

---

## 6. Conditions & Constraints

1. **Star Ground Topology:** Without a solid common ground between the power supply return, the buck converter, the Arduino GND pin, the TB6612FNG GND pins, and the sensor modules, logic reference voltages float and H-bridge gates misfire.
2. **Motor Inrush Current vs. Power Delivery:** Geared DC motors draw up to $1.5\text{A} - 2.5\text{A}$ in locked-rotor stall / initial inrush current. Standard wall-wart adapters ($1.2\text{A} - 1.5\text{A}$) collapse their output voltage rail when both motors engage simultaneously, resetting the Arduino.
3. **Logic Supply Protection:** The ATmega328P `5V` rail must never exceed $5.5\text{V}$ or receive reverse polarity. Pre-power-on multimeter verification is mandatory before connecting logic headers.

---

## 7. Accumulated Evidence & Empirical Experience

In strict accordance with the ecosystem's **Epistemic Integrity** standards, this section preserves the exact physical lessons discovered during initial testing:

### Finding 1: Standalone Motor Driver Gate Switching (Hardware Demystification)
* **Experiment:** The TB6612FNG was tested on a breadboard completely disconnected from the Arduino. Pins `STBY`, `PWMA`, and `AIN1` were tied directly to $+5\text{V}$, and `AIN2` to `GND`.
* **Discovery:** The motor spun at full speed immediately. This proved empirically that the microcontroller pins are not providing "magic power"; they are acting as simple solid-state current switches (logic gates) controlling the internal MOSFET H-bridge.

### Finding 2: Arduino Uno Hardware Health Audit
* **Experiment:** Tested two candidate Arduino Uno boards under Linux (`lsusb`, `dmesg`, `avrdude`).
* **Discovery:**
  - **Board #1 (Genuine Uno R3):** ATmega16U2 bridge enumerated cleanly (`2341:0043` on `/dev/ttyACM0`), Optiboot v4.4 responded in $0.00\text{s}$, device signature `0x1e950f` confirmed healthy. **Selected as primary robot compute.**
  - **Board #2 (Clone):** CP2102 bridge enumerated on `/dev/ttyUSB0`, but `avrdude` reported `not in sync: resp=0x00`. Diagnosis revealed the main ATmega328P had suffered reverse-polarity damage on its 5V rail, frying the internal ESD clamping diodes while leaving the USB chip alive.
  - Detailed diagnostic report preserved in [resources/arduino_diagnosis.md](file:///E:/Line%20follower%20project/resources/arduino_diagnosis.md).

### Finding 3: Closed-Loop Sensor-to-Motor Response (`LFR_V1.1.01.ino`)
* **Experiment:** Added front corner IR sensors and implemented reactive differential steering.
* **Discovery:** Verified that optical reflection transitions cleanly toggle the digital pins (`D2`/`D3`) and trigger the appropriate steering correction states.

### Finding 4: Power Supply Deficit & Inrush Collapse
* **Experiment:** Attempted full vehicle test using a 12V 1.5A wall adapter.
* **Discovery:** When both motors attempted to start simultaneously at `baseSpeed = 140`, the instantaneous inrush current tripped the adapter's internal over-current protection, dropping the voltage and browning out the system.
* **Engineering Decision:**
  1. *Immediate Firmware Patch:* Authored `LFR_V1.1.02_SoftStart.ino` implementing a staggered soft-start ramp (starting Motor A first, ramping PWM in steps of $+15$ every $15\text{ms}$, then engaging Motor B) to prevent overlapping inrush spikes.
  2. *Definitive Hardware Decision:* Ordered a dedicated high-current **Switched-Mode Power Supply (SMPS)** capable of delivering sustained motor stall current without voltage sag.

---

## 8. Current Status & Next Review

- **Current Status:** **Active (Awaiting SMPS Delivery)**
- **Next Milestone:** SMPS Power Integration & Floor Lane-Tracking Verification.
- **Immediate Action Plan Upon SMPS Delivery:**
  1. Measure SMPS output voltage with multimeter and calibrate buck converter to $+8.0\text{V}$ (into Arduino `VIN`).
  2. Perform pre-power continuity and ground check according to [resources/WIRING_DIAGRAM_V1.1.01.md](file:///E:/Line%20follower%20project/resources/WIRING_DIAGRAM_V1.1.01.md).
  3. Flash `LFR_V1_1_01_DualBoundary.ino` to Board #1 and run track trials.
  4. Record track departure speeds to empirically measure the phase lag $\tau$ and limit-cycle oscillation predicted in [Article LFE-02](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/02%20%E2%80%94%20Domains/ROS2/ROS2%20Articles/07_Line_Follower_Evolution_Series/article_lfe_02_v1_reactive_physics_and_instability.md).

---

## 9. Relationships & Resources

- **Schematic & Pin Tables:** [resources/WIRING_DIAGRAM_V1.1.01.md](file:///E:/Line%20follower%20project/resources/WIRING_DIAGRAM_V1.1.01.md)
- **Board Health Diagnostics:** [resources/arduino_diagnosis.md](file:///E:/Line%20follower%20project/resources/arduino_diagnosis.md)
- **Historical Migration Log:** [resources/PROJECT_REPORT_HISTORICAL.md](file:///E:/Line%20follower%20project/resources/PROJECT_REPORT_HISTORICAL.md)
- **Ecosystem Project Anchor:** [03 — Projects & Experiments/Active/Line Follower Robot](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/03%20%E2%80%94%20Projects%20&%20Experiments/Active/Line%20Follower%20Robot/README.md)
- **Simulation Counterpart:** [Line_Follower_Evolution_ws](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/02%20%E2%80%94%20Domains/ROS2/Line_Follower_Evolution_ws/README.md)
- **Curriculum Series:** [ROS2 Articles / 07_Line_Follower_Evolution_Series](file:///e:/Intelligent%20Systems%20Knowledge%20Ecosystem/02%20%E2%80%94%20Domains/ROS2/ROS2%20Articles/07_Line_Follower_Evolution_Series/article_lfe_01_evolutionary_intelligence_framework.md)
