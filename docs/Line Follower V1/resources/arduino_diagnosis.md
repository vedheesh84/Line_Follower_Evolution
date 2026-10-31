# Arduino Uno R3 Diagnostic & Verification Guide

**Date:** October 24, 2026  
**Host System:** Ubuntu 22.04 LTS (Kernel `6.8.0-138-generic`)  
**Target Board:** Arduino Uno R3 (ATmega328P)

---

## Summary of Findings

| Check | Result | Details |
| :--- | :--- | :--- |
| **USB Enumeration** | **PASS** | `ID 2341:0043 Arduino SA Uno R3 (CDC ACM)` on Bus `003` Device `031` |
| **Serial Number** | **PASS** | `3423433353835141C1C0` |
| **Device Node** | **PASS** | `/dev/ttyACM0` |
| **Persistent Symlink** | **PASS** | `/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_3423433353835141C1C0-if00` |
| **User Permissions** | **PASS** | User `ved` is in the `dialout` group (`gid=20`); no `sudo` required |
| **DTR Auto-Reset & Bootloader** | **PASS** | Optiboot v4.4 (`Hardware Version: 3`, `Firmware Version: 4.4`) responded in `0.00s` |
| **MCU Signature** | **PASS** | `0x1e950f` (ATmega328P — 32 KB Flash, 1 KB EEPROM) |
| **Active Serial Output** | **IDLE** | No serial output (`b''`) at `9600` or `115200` baud |

---

## Step-by-Step Diagnostic Process

### Step 1: Verify USB Bus Enumeration (`lsusb`)
* `2341:0043` is the official USB Vendor ID (`0x2341` = Arduino SA) and Product ID (`0x0043` = Uno R3).
* `CDC ACM` confirms the onboard ATmega16U2 USB-to-Serial bridge is enumerating properly as a standard USB modem/serial device.

### Step 2: Check Serial Device Node & User Permissions
* `/dev/ttyACM0` has permissions `crw-rw----` owned by user `root` and group `dialout`.
* The `id` output shows `20(dialout)` in user `ved`'s group list, granting direct access to upload sketches and open the Serial Monitor.
* `/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_3423433353835141C1C0-if00` is a persistent path tied to the board's unique serial number (`3423433353835141C1C0`).

### Step 3: Inspect Kernel USB & Driver Logs (`journalctl -k`)
* At `21:16:21`, the port locked in cleanly at full-speed (`12 Mbps`) as Device `31` and bound to the `cdc_acm` driver as `ttyACM0`.

### Step 4: Verify ATmega328P Microcontroller & Bootloader (`avrdude`)
* **DTR Auto-Reset:** Pulsing DTR successfully reset the ATmega328P into bootloader mode.
* **Bootloader Version:** `Firmware Version: 4.4` confirms standard Optiboot v4.4 is intact.
* **Chip Signature:** `0x1e950f` is the hardware silicon signature for the **ATmega328P** (`0x1E` = Atmel/Microchip, `0x95` = 32 KB Flash, `0x0F` = ATmega328P).

---

## Board #2 Diagnosis: Detecting MCU Failure (Reversed 5V Polarity Damage)

When swapping to the second board on the same USB port, initial USB detection succeeded, but deeper protocol and multi-baud probing isolated a dead main microcontroller caused by reverse-polarity 5V power damage.

### Summary of Board #2 Findings

| Check | Result | Details |
| :--- | :--- | :--- |
| **USB-to-UART Bridge** | **PASS (Degraded)** | `ID 10c4:ea60 Silicon Labs CP210x UART Bridge` (`CP2102`, Serial `0001`) on `/dev/ttyUSB0` |
| **USB Enumeration Stability** | **WARN** | Failed 4 enumeration attempts (`error -71` / `unable to enumerate USB device`) over 12s before connecting |
| **Optiboot Handshake (`115200` baud)** | **FAIL** | `stk500_recv(): programmer is not responding` / `not in sync: resp=0x00` |
| **Fallback Bootloader Sweep (`57600`, `wiring`)** | **FAIL** | No response across Old Bootloader (`57600`) or Mega2560 (`wiring`) protocols |
| **Raw STK500v1 & Boot ROM Sweep** | **FAIL** | Zero bytes (`b''`) returned across `9600`, `19200`, `57600`, `74880`, and `115200` baud |

### Root Cause Conclusion (Electrical Analysis)
1. **Why `/dev/ttyUSB0` still appears in Linux:** The **CP2102 USB-to-UART chip** has its own internal 3.3V regulator powered directly from the 5V `VBUS` pin of the USB cable (or survived the reverse-polarity event), allowing it to enumerate over USB when plugged into the PC.
2. **Why all MCU probes return `0x00` / `b''`:** Feeding reversed-polarity 5V directly into the board's `5V` rail bypasses the barrel-jack reverse-protection diode and forward-biases the internal ESD clamping diodes of the main microcontroller from `GND` to `VCC`, destroying the MCU while leaving the USB bridge partially functional.
