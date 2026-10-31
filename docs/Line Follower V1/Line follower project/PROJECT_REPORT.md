# Line Follower Project — Audit, Cleanup & Status Report

**Date:** October 24, 2026  
**Project Directory:** `/home/ved/Line follower project`

---

## 1. Why the Folder Was Cluttered (Root Cause)

An inspection of file timestamps (`Oct 24 19:00–19:01`) and symlink targets revealed that the folder had been populated by **copy-pasting the results of a file-manager search for the substring `"line"`** across `/home/ved`.

Because the search matched any file or folder containing `"line"` in its name:
1. **Unrelated projects were pulled in:**
   - `non`**`line`**`ar_dynamics.py`, `nonlinear_dynamics` (symlink), and `nonlinear_dynamics.cpython-310.pyc` from `drone_ws/src/controller/controller/`.
   - `mapper_params_on`**`line`**`_async.yaml` (6 duplicate copies/symlinks) from `SLAM` / `articubot_one` / `mobile_manipulator_ws`.
   - `Untitled Document 1` — a saved Claude Code conversation log from `gripper_car_ws` (which happened to match during the copy).
2. **ROS 2 `build/`, `install/`, `log/`, and `src/` folders with the same package names (`line_follower_robot` and `simple_linefollower_robot`) were merged on top of each other:**
   - Merging `/home/ved/ros2_doc/gazebo_ws/{src,build,install,log}/line_follower_robot` into `/home/ved/Line follower project/line_follower_robot` dumped CMake build artifacts (`CMakeCache.txt`, `Makefile`, `ament_cmake_*`, `colcon_command_prefix_*`) and broken absolute symlinks directly into the package root alongside `CMakeLists.txt` and `package.xml`.
3. **18 loose symlinks & build artifacts** (`package.xml`, `line_follower_robot.sdf`, `line_follower_robotConfig*.cmake`, `wheel_publisher.cpython-310.pyc`, `*_uninstall.dir`, etc.) were dropped into the top-level directory.

---

## 2. Cleanup Performed

### Removed Clutter & Corrupted Artifacts
- Deleted all **18 loose top-level symlinks, `.pyc` files, duplicate YAMLs, and CMake `.dir` folders**.
- Deleted unrelated files: `nonlinear_dynamics.py`, `nonlinear_dynamics`, `nonlinear_dynamics.cpython-310.pyc`, `mapper_params_online_async.yaml` (`1`–`5`), and `Untitled Document 1`.
- Replaced the corrupted, merged `line_follower_robot/` and `simple_linefollower_robot/` directories with clean, self-contained source copies from `/home/ved/ros2_doc/gazebo_ws/src/` (and stripped `__pycache__`).

### Current Clean Directory Structure
```text
/home/ved/Line follower project/
├── PROJECT_REPORT.md                  # This project audit and status report
├── arduino_diagnosis.md               # Hardware diagnostic report for Arduino Boards #1 and #2
├── line follower/                     # Arduino firmware sketches
│   ├── LFR_V1.0/
│   │   └── LFR_V1.0.ino               # Line Follower Robot motor driver test sketch (Arduino Uno)
│   └── mobile_base_controller/
│       └── mobile_base_controller.ino # 4-wheel PID + encoder controller (from gripper_car_ws, Arduino Mega)
├── line_follower_robot/               # ROS 2 (ament_cmake) full CAD mesh URDF/SDF/Gazebo package
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── package.xml
│   ├── config/joint_names_line robt follower.yaml
│   ├── launch/                        # 7 ROS 2 launch files (Gazebo, RViz, control, debug)
│   ├── meshes/                        # 5 SolidWorks STL meshes (base, wheels, castor)
│   ├── models/line_follower_robot.sdf # Standalone Gazebo SDF model
│   ├── rviz/urdf.rviz                 # RViz2 configuration
│   ├── script/wheel_publisher.py      # Python joint_states test publisher
│   └── urdf/                          # Xacro & URDF robot descriptions
├── simple_linefollower_robot/         # ROS 2 (ament_cmake) simplified parametric Xacro package
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── package.xml
│   ├── launch/                        # 6 ROS 2 launch files
│   ├── rviz/urdf.rviz
│   └── urdf/line_follower_simple_robot.urdf.xacro
└── line robt follower.zip             # Original SolidWorks-to-URDF ROS 1 export archive (Sep 2025)
```

---

## 3. Component-by-Component Analysis & Status

### A. Arduino Firmware (`line follower/`)

#### 1. `line follower/LFR_V1.0/LFR_V1.0.ino` — **Target Line Follower Sketch (Incomplete)**
- **Hardware Target:** Arduino Uno R3 + **TB6612FNG** dual DC motor driver.
- **Pin Mapping:**
  - Left Motor (Channel A): `PWMA = D5`, `AIN1 = D8`, `AIN2 = D9`
  - Right Motor (Channel B): `PWMB = D6`, `BIN1 = D10`, `BIN2 = D11`
  - Driver Standby: `STBY = D7` (pulled `HIGH` in `setup()`)
- **Implemented Functions:** `forward()`, `backward()`, `turnLeft()`, `turnRight()`, `stopMotors()`.
- **Current Status:** **Open-loop motor test only.** `loop()` simply cycles through `forward(150)` -> `backward(150)` -> `turnLeft(120)` -> `turnRight(120)` -> `stopMotors()` with 2-second delays.
- **Missing for Line Following:**
  - No IR sensor array pin definitions (`A0`–`A4` or digital inputs).
  - No sensor reading/calibration routine.
  - No line-following control algorithm (bang-bang or PID steering using `analogWrite` on `PWMA`/`PWMB`).

#### 2. `line follower/mobile_base_controller/mobile_base_controller.ino` — **Unrelated Mobile Manipulator Firmware**
- **Origin:** Copied from `/home/ved/Arduino/line follower/mobile_base_controller`, originally written for `gripper_car_ws`.
- **Hardware Target:** **Arduino Mega 2560** driving 4 DC motors via 2x L298N drivers (`pins 22–29`, `PWM 4–7`) with 4 quadrature encoders (`pins 18, 19, 2, 3, 50–53`).
- **Note:** This sketch will **not** run on the Arduino Uno (uses Mega-only pins 22–53 and 4 hardware interrupts), though its PID implementation can serve as a reference.

---

### B. ROS 2 Simulation Packages

#### 1. `line_follower_robot` (CAD Mesh Package)
- **Origin:** Migrated from the SolidWorks URDF export (`line robt follower.zip`, exported Sep 26, 2025) into a ROS 2 `ament_cmake` package.
- **Robot Geometry:**
  - Differential-drive chassis (`base_link.STL`) with `left_wheel` and `right_wheel` (`continuous` joints) plus a passive front/rear caster assembly (`castor_base_link` + `castor_wheel`, both `continuous`).
- **Launch Files:**
  - `display.launch.py` / `rviz.launch.py`: Launches `robot_state_publisher`, `joint_state_publisher_gui`, and `rviz2`.
  - `gazebo.launch.py`: Launches Gazebo (`gazebo_ros`) and spawns the robot from `/robot_description`.
  - `control.launch.py`: Starts `wheel_publisher.py` to animate wheel joints.
- **Bugs & Gaps Identified:**
  1. **Robot is welded in mid-air (`world_joint`):** In `urdf/line_follower_robot.urdf.xacro` (lines 17–22), `base_link` is attached to `world` via `<joint name="world_joint" type="fixed">` at `z = 1.0 m`. Because it is a `fixed` joint to `world`, the robot cannot move or fall onto the ground plane in Gazebo.
  2. **Joint name mismatch in `script/wheel_publisher.py`:** Line 25 publishes joint names `['left_wheel', 'right_wheel']` (which are link names) instead of the actual joint names `['left_wheel_joint', 'right_wheel_joint']`. As a result, `robot_state_publisher` ignores the messages from `control.launch.py`.
  3. **Wrong collision mesh on `castor_base_link`:** In both `urdf/line_follower_robot.urdf.xacro` (line 137) and `models/line_follower_robot.sdf` (line 76), `castor_base_link`'s `<collision>` geometry points to `castor_wheel.STL` instead of `castor_base_link.STL`.
  4. **Missing Gazebo plugins & line sensor:** There is no `libgazebo_ros_diff_drive.so` plugin (to accept `/cmd_vel` commands) and no camera/ray sensor plugin or ROS 2 line-follower node to detect a line in simulation.

#### 2. `simple_linefollower_robot` (Parametric Xacro Package)
- **Purpose:** Clean, lightweight version of the robot using parametric Xacro macros (`cylinder_inertia`, `box_inertia`, `sphere_inertia`, `wheel` macro) instead of heavy CAD inertia matrices.
- **Bugs & Gaps Identified:**
  - Also has `<joint name="world_joint" type="fixed">` in `urdf/line_follower_simple_robot.urdf.xacro` (lines 62–67), locking `base_link` to `world`.
  - Missing Gazebo diff-drive and line-sensor plugins.

---

## 4. Recommended Next Steps

1. **Hardware / Arduino (`LFR_V1.0.ino`):**
   - Use **Board #1** (the healthy genuine Arduino Uno R3 diagnosed in `arduino_diagnosis.md`).
   - Add the IR sensor array pin definitions and implement a PID or proportional line-following loop in `LFR_V1.0.ino`.
2. **ROS 2 Simulation (`line_follower_robot`):**
   - Remove the fixed `world_joint` (or make it optional via a Xacro argument) so the robot rests on the ground and can drive in Gazebo.
   - Fix `wheel_publisher.py` joint names (`left_wheel_joint`, `right_wheel_joint`) and fix `castor_base_link`'s collision mesh (`castor_base_link.STL`).
   - Add `libgazebo_ros_diff_drive.so` and a downward-facing camera/IR sensor plugin to simulate closed-loop line following in Gazebo.
   
   
   agy --conversation=03e8e4f1-2f1b-4e8c-8bd5-3dbf97c9ea09

