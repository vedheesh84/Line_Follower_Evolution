# Project: Line Follower Robot (V1 Physical Embodiment)

**Entity Type:** Active Hardware Project & Physical Proving Ground  
**Scope:** `03 — Projects & Experiments/Active/Line Follower Robot`  
**Physical Working Directory:** [`Line Follower V1/`](<Line Follower V1/README.md>)  
**Status:** Active (Awaiting SMPS Power Supply)  
**Primary Domain:** [ROS2](<../../../02 — Domains/ROS2/README.md>) / Robotics  
**Simulation Counterpart:** [Line_Follower_Evolution_ws](<../../../02 — Domains/ROS2/Line_Follower_Evolution_ws/README.md>) ([GitHub](https://github.com/vedheesh84/ros2-knowledge-ecosystem/tree/main/Line_Follower_Evolution_ws))  
**Theoretical Reference:** [Article LFE-02: V1 Reactive Physics & Real-World Instability](<../../../02 — Domains/ROS2/ROS2 Articles/07_Line_Follower_Evolution_Series/article_lfe_02_v1_reactive_physics_and_instability.md>)  

---

## 1. Overview

The **Line Follower Robot (Physical V1)** is the physical hardware proving ground for Generation 1 of the robotic evolution roadmap within the Intelligent Systems Knowledge Ecosystem.

While [Line_Follower_Evolution_ws](<../../../02 — Domains/ROS2/Line_Follower_Evolution_ws/README.md>) explores this generation algorithmically in ROS 2 simulation, this physical build confronts the real-world constraints of discrete hardware: logic voltage switching, back-EMF, motor inrush current spikes, power supply brownouts, sensor thresholding, and bang-bang steering dynamics.

---

## 2. Intent

To design, assemble, program, and empirically validate a 2-sensor differential drive robot on physical hardware, demonstrating:
1. Physical gate-level H-bridge control using the TB6612FNG driver and Arduino Uno R3.
2. The real-world behavior and instability of bang-bang thresholding over an S-curve track.
3. Electrical power isolation and common-ground discipline across motor and logic rails.

---

## 3. Physical Architecture & Components

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│                           LOGIC & SENSING LAYER                              │
│   Arduino Uno R3 (ATmega328P)  <───  2x LM393 IR Reflection Sensors (D2, D3)│
└──────────────────────────────────────┬──────────────────────────────────────┘
                                       │ PWM (D5, D6) + Direction (D8-D11)
┌──────────────────────────────────────▼──────────────────────────────────────┐
│                           ACTUATION LAYER                                    │
│   TB6612FNG Dual MOSFET H-Bridge Driver (Straddling Breadboard Trench)       │
└──────────────────────────────────────┬──────────────────────────────────────┘
                                       │ Motor A (Left) & Motor B (Right)
┌──────────────────────────────────────▼──────────────────────────────────────┐
│                           PHYSICAL CHASSIS                                   │
│   2WD Differential Drive Chassis + 2x Geared DC Motors + Front Caster Wheel  │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 4. Key Empirical Lessons & Discoveries

1. **Hardware Switching Verification:** Manually tying TB6612FNG control pins (`STBY`, `PWMA`, `AIN1`) to 5V and `AIN2` to GND confirmed that driver inputs function strictly as solid-state logic gates rather than providing drive current.
2. **Board Integrity Isolation:** Diagnostic testing identified a dead clone board (burned MCU ESD diodes from reversed 5V) versus a healthy genuine Uno R3 (Optiboot 4.4 confirmed).
3. **Inrush Current Tripping:** Starting both geared motors simultaneously on a 12V 1.5A adapter caused voltage collapse. A firmware soft-start ramp was implemented while a dedicated high-current SMPS was ordered.

---

## 5. Ecosystem Navigation & Resources

- **Physical Build Directory:** [`Line Follower V1/README.md`](<Line Follower V1/README.md>)
- **Wiring Schematic & Power Tables:** [`Line Follower V1/resources/WIRING_DIAGRAM_V1.1.01.md`](<Line Follower V1/resources/WIRING_DIAGRAM_V1.1.01.md>)
- **Arduino Diagnostic Report:** [`Line Follower V1/resources/arduino_diagnosis.md`](<Line Follower V1/resources/arduino_diagnosis.md>)
- **Firmware Sketches:** [`Line Follower V1/firmware/`](<Line Follower V1/firmware>)

---

## 6. Knowledge & Research Bridge (Obsidian 2nd-Brain Vault)

Personal hardware learnings, component notes, and early roadmap synthesis for this build live in the Obsidian Vault (`ved`):

- **Obsidian Project Concept Note:** [Line Follower V1.md](file:///e:/app%20data/obsidian/ved/40_Projects/Robotics%20Learning%20Projects/Line%20follower/Line%20Follower%20V1.md) (`obsidian://open?vault=ved&file=40_Projects%2FRobotics%20Learning%20Projects%2FLine%20follower%2FLine%20Follower%20V1`)
- **Project MOC Hub:** [Project MOC - ROS 2 Learning Kit](file:///e:/app%20data/obsidian/ved/05_MOCs/Project_MOCs/Project%20MOC%20-%20ROS%202%20Learning%20Kit.md)
- **Related Atomic Notes (`30_Notes/`):** `[[TB6612FNG]]`, `[[DC Motors]]`, `[[H-Bridge]]`, `[[Simple IR Line Follower Arduino Code]]`
