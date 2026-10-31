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

**Purpose:** Confirm that the Linux kernel and USB host controller detect the Arduino's USB-to-Serial converter chip (ATmega16U2 on genuine Uno R3 boards, or CH340/CP2102 on clones).

**Command:**
```bash
lsusb
```

**Output:**
```text
Bus 004 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 003 Device 003: ID 04ca:707f Lite-On Technology Corp. HP Wide Vision HD Camera
Bus 003 Device 031: ID 2341:0043 Arduino SA Uno R3 (CDC ACM)
Bus 003 Device 004: ID 0bda:385a Realtek Semiconductor Corp. Bluetooth Radio
Bus 003 Device 002: ID 1ea7:0064 SHARKOON Technologies GmbH 2.4GHz Wireless rechargeable vertical mouse [More&Better]
Bus 003 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
```

**Explanation:**
* `2341:0043` is the official USB Vendor ID (`0x2341` = Arduino SA) and Product ID (`0x0043` = Uno R3).
* `CDC ACM` (Communications Device Class - Abstract Control Model) confirms the onboard ATmega16U2 USB-to-Serial bridge is enumerating properly as a standard USB modem/serial device.

---

### Step 2: Check Serial Device Node & User Permissions

**Purpose:** Verify that `udev` created the character device `/dev/ttyACM0`, locate its persistent hardware symlink, and confirm the current user has read/write access without requiring `sudo`.

**Command:**
```bash
ls -la /dev/ttyACM* /dev/ttyUSB* /dev/serial/by-id/* 2>/dev/null
id
```

**Output:**
```text
lrwxrwxrwx 1 root root        13 Oct 24 21:16 /dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_3423433353835141C1C0-if00 -> ../../ttyACM0
crw-rw---- 1 root dialout 166, 0 Oct 24 21:16 /dev/ttyACM0
uid=1000(ved) gid=1000(ved) groups=1000(ved),4(adm),20(dialout),24(cdrom),27(sudo),30(dip),46(plugdev),122(lpadmin),135(lxd),136(sambashare)
```

**Explanation:**
* `/dev/ttyACM0` has permissions `crw-rw----` owned by user `root` and group `dialout`.
* The `id` output shows `20(dialout)` in user `ved`'s group list, granting direct access to upload sketches and open the Serial Monitor.
* `/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_3423433353835141C1C0-if00` is a persistent path tied to the board's unique serial number (`3423433353835141C1C0`), useful in ROS 2 launch files or Python scripts so the port never changes even if other USB devices are plugged in.

---

### Step 3: Inspect Kernel USB & Driver Logs (`journalctl -k`)

**Purpose:** Check the kernel ring buffer for physical connection stability, driver binding (`cdc_acm`), and ensure no rogue services (such as `brltty` braille daemon or `ModemManager`) hijacked the port.

**Command:**
```bash
journalctl -k -n 30 --no-pager | grep -iE "usb|tty|acm|ch34|ftdi|cp210|serial|brltty"
```

**Output:**
```text
Oct 24 21:16:19 vedheesh kernel: usb 3-2: new low-speed USB device number 28 using xhci_hcd
Oct 24 21:16:20 vedheesh kernel: usb 3-2: device descriptor read/64, error -71
Oct 24 21:16:20 vedheesh kernel: usb 3-2: device descriptor read/64, error -71
Oct 24 21:16:20 vedheesh kernel: usb 3-2: new low-speed USB device number 29 using xhci_hcd
Oct 24 21:16:20 vedheesh kernel: usb usb3-port2: attempt power cycle
Oct 24 21:16:20 vedheesh kernel: usb 3-2: new high-speed USB device number 30 using xhci_hcd
Oct 24 21:16:20 vedheesh kernel: usb 3-2: Device not responding to setup address.
Oct 24 21:16:21 vedheesh kernel: usb 3-2: Device not responding to setup address.
Oct 24 21:16:21 vedheesh kernel: usb 3-2: device not accepting address 30, error -71
Oct 24 21:16:21 vedheesh kernel: usb 3-2: new full-speed USB device number 31 using xhci_hcd
Oct 24 21:16:21 vedheesh kernel: usb 3-2: New USB device found, idVendor=2341, idProduct=0043, bcdDevice= 0.01
Oct 24 21:16:21 vedheesh kernel: usb 3-2: New USB device strings: Mfr=1, Product=2, SerialNumber=220
Oct 24 21:16:21 vedheesh kernel: usb 3-2: Manufacturer: Arduino (www.arduino.cc)
Oct 24 21:16:21 vedheesh kernel: usb 3-2: SerialNumber: 3423433353835141C1C0
Oct 24 21:16:21 vedheesh kernel: cdc_acm 3-2:1.0: ttyACM0: USB ACM device
```

**Explanation:**
* Between `21:16:19` and `21:16:21`, as the USB connector was physically inserted into port `3-2`, the contacts bounced briefly (`error -71` / `EPROTO`), which is normal if inserted slowly or if the USB-B connector is slightly loose.
* At `21:16:21`, the port locked in cleanly at full-speed (`12 Mbps`) as Device `31` and bound to the `cdc_acm` driver as `ttyACM0`.

---

### Step 4: Verify ATmega328P Microcontroller & Bootloader (`avrdude`)

**Purpose:** Even if `/dev/ttyACM0` shows up, the main ATmega328P chip or its auto-reset capacitor/bootloader could be damaged. Using `avrdude` with `-n` (no-write / read-only test mode) pulses the DTR line to reset the ATmega328P, handshakes with the Optiboot bootloader at `115200` baud, and reads the silicon device signature.

**Command:**
```bash
AVRDUDE_BIN="/home/ved/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/bin/avrdude"
AVRDUDE_CONF="/home/ved/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/etc/avrdude.conf"

"$AVRDUDE_BIN" -C "$AVRDUDE_CONF" -p atmega328p -c arduino -P /dev/ttyACM0 -b 115200 -n -v
```

**Key Flags Explained:**
* `-C "$AVRDUDE_CONF"`: Path to the AVR chip configuration database.
* `-p atmega328p`: Target microcontroller (ATmega328P on the Arduino Uno).
* `-c arduino`: Programmer protocol (`stk500v1` with DTR/RTS auto-reset pulse).
* `-P /dev/ttyACM0`: Serial port device node.
* `-b 115200`: Baud rate used by the Uno Optiboot bootloader.
* `-n`: **No-write mode** (strictly read-only test; does not modify Flash or EEPROM).
* `-v`: Verbose output showing hardware/firmware versions and memory layout.

**Output:**
```text
         Programmer Type : Arduino
         Description     : Arduino
         Hardware Version: 3
         Firmware Version: 4.4
         Vtarget         : 0.3 V
         Varef           : 0.3 V
         Oscillator      : 28.800 kHz
         SCK period      : 3.3 us

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e950f (probably m328p)
avrdude: safemode: Fuses OK (E:00, H:00, L:00)

avrdude done.  Thank you.
```

**Explanation:**
* **DTR Auto-Reset:** Pulsing DTR successfully reset the ATmega328P into bootloader mode.
* **Bootloader Version:** `Firmware Version: 4.4` confirms standard Optiboot v4.4 is intact.
* **Chip Signature:** `0x1e950f` is the hardware silicon signature for the **ATmega328P** (`0x1E` = Atmel/Microchip, `0x95` = 32 KB Flash, `0x0F` = ATmega328P).

---

### Step 5: Check Active Serial Output (`pyserial`)

**Purpose:** Listen on `/dev/ttyACM0` at standard baud rates (`9600` and `115200`) to see if the currently flashed sketch is streaming telemetry or sensor readings.

**Command:**
```bash
python3 -c '
import serial, time
for baud in [9600, 115200]:
    with serial.Serial("/dev/ttyACM0", baud, timeout=1.5) as s:
        time.sleep(2.0)
        data = s.read(256)
        print(f"Baud {baud}: {data!r}")
'
```

**Output:**
```text
Baud 9600: b''
Baud 115200: b''
```

**Explanation:**
* Note that opening `/dev/ttyACM0` resets the Arduino Uno, so `time.sleep(2.0)` waits for the 1.5-second Optiboot bootloader window to finish before reading.
* Both baud rates returned `b''` (empty bytes), indicating the currently loaded sketch does not print to `Serial` on startup.

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

---

### Additional Diagnostic Steps & Findings

#### 1. Identifying the Different USB Bridge & Enumeration Instability

Unlike Board #1 (`2341:0043` on `/dev/ttyACM0`), `lsusb` and `journalctl -k` showed that Board #2 uses a **Silicon Labs CP2102** USB-to-UART bridge attached to `/dev/ttyUSB0`, and struggled during initial USB enumeration:

```text
Bus 003 Device 042: ID 10c4:ea60 Silicon Labs CP210x UART Bridge
lrwxrwxrwx 1 root root 13 Oct 24 21:28 /dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0 -> ../../ttyUSB0
crw-rw-rw- 1 root dialout 188, 0 Oct 24 21:28 /dev/ttyUSB0
```

**Kernel Enumeration Log (`journalctl -k`):**
```text
Oct 24 21:28:43 vedheesh kernel: usb 3-2: Device not responding to setup address.
Oct 24 21:28:43 vedheesh kernel: usb 3-2: device not accepting address 40, error -71
Oct 24 21:28:44 vedheesh kernel: usb 3-2: WARN: invalid context state for evaluate context command.
Oct 24 21:28:44 vedheesh kernel: usb usb3-port2: unable to enumerate USB device
Oct 24 21:28:53 vedheesh kernel: usb 3-2: New USB device found, idVendor=10c4, idProduct=ea60, bcdDevice= 1.00
Oct 24 21:28:53 vedheesh kernel: usb 3-2: Product: CP2102 USB to UART Bridge Controller
Oct 24 21:28:53 vedheesh kernel: usb 3-2: cp210x converter now attached to ttyUSB0
```

---

#### 2. Bootloader Handshake Failure on `/dev/ttyUSB0`

Running `avrdude` against `/dev/ttyUSB0` at `115200` baud failed to receive any reply from the microcontroller:

```text
Using Port                    : /dev/ttyUSB0
Using Programmer              : arduino
Overriding Baud Rate          : 115200
avrdude: stk500_recv(): programmer is not responding
avrdude: stk500_getsync() attempt 1 of 10: not in sync: resp=0x00
```

---

#### 3. Multi-Protocol & Multi-Baud Fallback Probing

To rule out a different bootloader speed or board architecture behind the CP2102 bridge, three targeted follow-up tests were executed:

##### A. DTR/RTS Reset Pulse & Boot ROM Listener
**Purpose:** Pulse the hardware `DTR` and `RTS` reset lines while listening at common microcontroller boot ROM baud rates (`115200`, `74880`, `57600`, `9600`).

**Command:**
```bash
python3 -c '
import serial, time

port = "/dev/ttyUSB0"
for baud in [115200, 74880, 9600, 57600]:
    with serial.Serial(port, baud, timeout=0.8) as s:
        s.dtr = False
        s.rts = True
        time.sleep(0.1)
        s.rts = False
        time.sleep(0.5)
        data = s.read(512)
        print(f"[Boot ROM / Reset Output @ {baud}]: {data if data else \"empty\"}")
'
```

**Output:**
```text
[Boot ROM / Reset Output @ 115200]: empty
[Boot ROM / Reset Output @ 74880]: empty
[Boot ROM / Reset Output @ 9600]: empty
[Boot ROM / Reset Output @ 57600]: empty
```

##### B. Fallback Bootloader Protocols (`57600` Old Bootloader & `wiring`)
**Purpose:** Test whether the MCU uses the legacy ATmega328P bootloader (`57600` baud) or the STK500v2 `wiring` protocol.

**Command:**
```bash
timeout 3 "$AVRDUDE_BIN" -C "$AVRDUDE_CONF" -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -n
timeout 3 "$AVRDUDE_BIN" -C "$AVRDUDE_CONF" -p atmega2560 -c wiring -P /dev/ttyUSB0 -b 115200 -D -n
```

**Result:** Both timed out with no response from the target MCU.

##### C. Raw STK500v1 Sync Byte Probe (`0x30 0x20`)
**Purpose:** Manually toggle `DTR`/`RTS` and transmit raw STK500v1 `Cmnd_STK_GET_SYNC` (`0x30`) + `Sync_CRC_EOP` (`0x20`) packets across all standard AVR baud rates (`115200`, `57600`, `19200`, `9600`). A healthy AVR bootloader replies with `b'\x14\x10'` (`STK_INSYNC` + `STK_OK`).

**Command:**
```bash
python3 -c '
import serial, time

for baud in [115200, 57600, 19200, 9600]:
    with serial.Serial("/dev/ttyUSB0", baud, timeout=0.3) as s:
        s.dtr = False; s.rts = False; time.sleep(0.05)
        s.dtr = True; s.rts = True; time.sleep(0.05)
        s.dtr = False; s.rts = False; time.sleep(0.2)
        s.reset_input_buffer()
        for _ in range(3):
            s.write(b"\x30\x20")
            time.sleep(0.05)
        resp = s.read(64)
        print(f"STK500v1 @ {baud}: {resp!r}")
'
```

**Output:**
```text
STK500v1 @ 115200: b''
STK500v1 @ 57600: b''
STK500v1 @ 19200: b''
STK500v1 @ 9600: b''
```

---

### Root Cause Conclusion (Electrical Analysis)

1. **Why `/dev/ttyUSB0` still appears in Linux:** The **CP2102 USB-to-UART chip** has its own internal 3.3V regulator powered directly from the 5V `VBUS` pin of the USB cable (or survived the reverse-polarity event), allowing it to enumerate over USB when plugged into the PC.
2. **Why all MCU probes return `0x00` / `b''`:** Feeding reversed-polarity 5V directly into the board's `5V` rail bypasses the barrel-jack reverse-protection diode and forward-biases the internal ESD clamping diodes of the main microcontroller from `GND` to `VCC`, destroying the MCU (and/or onboard voltage regulator) while leaving the USB bridge partially functional.
