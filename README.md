## Automation and Monitoring System

This project includes an automation subsystem based on an ESP32 for temperature control and real-time process monitoring of the biodigester.

### Objective

The goal of the automation system is to:

- Monitor the internal conditions of the biodigester in real time
- Maintain the biodigester temperature within the target mesophilic range
- Upload sensor data via Wi-Fi to a web platform for remote monitoring
- Support future control logic and safety functions

---

## Main Variables Measured

The ESP32 acquires the following process variables:

- **Biodigester slurry (boñiga) temperature**
  Temperature of the cow manure mixture inside the main tank

- **Hot water tank temperature**
  Temperature of the water used in the external heating loop

Both temperatures are measured using DS18B20 waterproof probe sensors on a shared OneWire bus.

---

## General System Description

This automation system is based on an **ESP32-WROOM-32E** microcontroller connected to the sensors and to the heating control stage.

The system performs four main tasks:

1. Read sensors periodically
2. Control the heating system (heater and circulation pump)
3. Display real-time status on a local LCD screen
4. Send data through Wi-Fi to the monitoring website


---

## Control Strategy

The current control logic is based on hysteresis temperature control.

### Biodigester temperature control logic
- If biodigester temperature falls below the lower threshold, the heating system is activated
- If biodigester temperature rises above the upper threshold, the heating system is deactivated


### Example operating thresholds
- Heating ON: below 37.0 °C
- Heating OFF: above 38.0 °C

---

## Hardware

### Microcontroller
- **ESP32-WROOM-32E** (Espressif)

### Sensors
- **DS18B20 waterproof probe sensors (x2)** for biodigester slurry temperature and hot water tank temperature. Both sensors share a single OneWire bus and each is identified by its unique 64-bit ROM address.

### Display
- **20x4 LCD 2004A Module (5V Blue Screen)** with I2C interface, connected via a **bidirectional I2C level shifter** (3.3V ↔ 5V) for local real-time display

### Actuation
- **SSR (Solid State Relay)** for switching the water heater (AC load)
- **SSR (Solid State Relay)** for circulation pump control (AC load)
- Both SSRs are driven through **Optocoupler MOSFET Driver Modules** for 3.3V→5V level shifting and optical isolation from the ESP32

---

## Wiring Diagram

### GPIO Pin Assignment

| Component | Signal | ESP32 GPIO |
|-----------|--------|------------|
| DS18B20 (x2) | DATA | GPIO4 |
| LCD 2004A (I2C) | SDA | GPIO21 (via level shifter) |
| LCD 2004A (I2C) | SCL | GPIO22 (via level shifter) |
| Optocoupler Module - Heater | PWM | GPIO25 |
| Optocoupler Module - Pump | PWM | GPIO26 |

Available GPIOs for future expansion: GPIO5, GPIO18, GPIO19, GPIO34

> **Note:** Both DS18B20 sensors (biodigester + water tank) share the same OneWire data line (GPIO4). Each sensor is identified by its unique 64-bit ROM address. Run the address scan on first boot and configure the addresses in the source code.

### SSR Driving Circuit (Optocoupler MOSFET Driver Module)

Each SSR is driven through an Optocoupler MOSFET Driver Module that provides:
- 3.3V to 5V level shifting
- Optical isolation between ESP32 and power circuitry
- Reliable SSR triggering with sufficient voltage margin

```
Optocoupler MOSFET Driver Module (x2)
┌──────────────────────────────────────────────┐
│  Terminal    Connection                       │
│  ────────   ──────────────────────────────    │
│  PWM        ESP32 GPIO25 (heater) / GPIO26 (pump) │
│  GND        ESP32 GND                        │
│  DC+        5V (ESP32 VIN, shared rail)      │
│  DC-        GND (common)                     │
│  OUT+       SSR input (+)                    │
│  OUT-       SSR input (-)                    │
└──────────────────────────────────────────────┘

Signal flow:
ESP32 GPIO HIGH (3.3V) → Module PWM → Optocoupler ON → MOSFET ON
→ DC+ (5V) flows to OUT+ → SSR input receives 5V → SSR switches AC load ON
```

### I2C Level Shifter (LCD)

A bidirectional level shifter (BSS138-based module) is placed between the ESP32 (3.3V) and the LCD 2004A I2C backpack (5V) to ensure reliable I2C communication.

```
ESP32 3.3V side          Level Shifter          LCD 5V side
GPIO21 (SDA)  ←────────→  LV1 ── HV1  ←────────→  PCF8574 SDA
GPIO22 (SCL)  ←────────→  LV2 ── HV2  ←────────→  PCF8574 SCL
3.3V          ──────────→  LV               HV  ←──────────  5V
GND           ──────────→  GND             GND  ←──────────  GND
```

### Power Distribution

| Component | Voltage | Source |
|-----------|---------|--------|
| LCD 2004A | 5V | ESP32 VIN (5V) via +5V rail |
| Optocoupler Module 1 (DC+) | 5V | ESP32 VIN (5V) via +5V rail |
| Optocoupler Module 2 (DC+) | 5V | ESP32 VIN (5V) via +5V rail |
| Level Shifter (HV) | 5V | ESP32 VIN (5V) via +5V rail |
| DS18B20 (x2) | 3.3V | ESP32 3.3V |
| Level Shifter (LV) | 3.3V | ESP32 3.3V |

All 5V components share a common +5V rail branched from ESP32 VIN.
All GND pins are connected to a common ground rail.

### Wiring Notes

```
ESP32 VIN (5V) ─── Breadboard +5V Rail ──┬── LCD VCC (via level shifter HV)
                                          ├── Optocoupler Module 1 DC+
                                          ├── Optocoupler Module 2 DC+
                                          └── Level Shifter HV

ESP32 3.3V ──┬── DS18B20 VCC (Red wire, both sensors)
             ├── 4.7kΩ pull-up resistor ── DS18B20 DATA (White wire)
             └── Level Shifter LV

ESP32 GND ──────── Breadboard GND Rail ──┬── LCD GND
                                         ├── DS18B20 GND (Black wire, both sensors)
                                         ├── Optocoupler Module 1 GND + DC-
                                         ├── Optocoupler Module 2 GND + DC-
                                         └── Level Shifter GND

DS18B20 DATA (White wire, both sensors) ── GPIO4
```

> **Important:** A single 4.7kΩ pull-up resistor is required between DS18B20 VCC (Red, 3.3V) and DATA (White, GPIO4) for reliable OneWire communication. One resistor is sufficient for both sensors on the shared bus.

> **Important:** Use waterproof stainless steel probe DS18B20 sensors (SUS316 recommended) for the biodigester slurry environment.

---

## Local LCD Display

A 20x4 LCD (2004A, 5V Blue Screen) is connected to the ESP32 via I2C through a bidirectional level shifter to provide on-site monitoring without requiring network access.

### Information displayed on the LCD
- Line 1: Biodigester slurry temperature
- Line 2: Hot water tank temperature
- Line 3: Heating system status (ON / OFF)
- Line 4: Pump status (ON / OFF)

This allows operators to check the system status directly at the installation site.

---

## Data Transmission

The ESP32 connects to a local Wi-Fi network and uploads sensor values via HTTPS to a monitoring platform hosted on Railway (Flask + PostgreSQL).

### Data sent to the server
- Biodigester temperature
- Hot water tank temperature
- Heating system status
- Pump status
- Timestamp

### Security
- Wi-Fi credentials are stored in ESP32 NVS (Non-Volatile Storage), not hardcoded in source
- API communication uses HTTPS
- Server-side credentials (DATABASE_URL, API keys) are stored as Railway environment variables
- API endpoints are protected with API key authentication

---

## Monitoring Website

The monitoring platform receives the data transmitted by the ESP32 and displays the process variables in real time. It is hosted on Railway with Flask and PostgreSQL.

### Expected functions of the website
- Live display of biodigester temperature
- Live display of hot water tank temperature
- Heater and pump status visualization
- Historical data storage for later analysis
- Future alarm or notification system

---

## Enclosure and Environmental Protection

The system is designed for outdoor installation at a biodigester site:

- All electronics (ESP32, level shifter, optocoupler modules) are housed in an **IP65-rated waterproof enclosure** (ABS)
- Cable entry points use **cable glands** for waterproof wire pass-through
- **SSRs are mounted on aluminum heat sinks** attached to the enclosure exterior for thermal dissipation
- Thermal compound is applied between SSR and heat sink
- LCD is visible through an acrylic window in the enclosure
- Both DS18B20 sensors use waterproof stainless steel probes

---

## Current Scope of the Automation Subsystem

At this stage, the automation subsystem is defined to include:

- Real-time sensing of key variables (2 DS18B20 temperature sensors on shared OneWire bus)
- Basic automatic temperature control with hysteresis
- Optically isolated SSR driving for AC load switching
- I2C level shifting for reliable LCD communication
- Wi-Fi communication with the monitoring website (HTTPS)
- Data logging in PostgreSQL for process evaluation

---

## Future Improvements

Possible future upgrades include:

- Safety shutdown in case of sensor failure
- Maximum water temperature protection
- Remote setpoint adjustment from the website
- Alarm system for abnormal temperature values
- Full dashboard with historical graphs
- pH sensor integration for fermentation monitoring (GPIO5, 18, or 19 available)

---

## Initial Reference Logic

An initial version of the control logic was first developed in Arduino-style code (see `proto.cpp`) using a MAX6675 sensor and MOSFET-based hysteresis control.
This logic has been migrated and expanded to the ESP32 platform in `biogester.cpp`, adding:

- Two DS18B20 temperature sensors (address-based identification on shared OneWire bus)
- Optocoupler MOSFET driver modules for SSR isolation
- I2C level shifter for LCD
- Wi-Fi communication
- Data upload to the web platform

---

## Summary

The automation subsystem is designed to turn the biodigester into a monitored and temperature-controlled system, using an ESP32-WROOM-32E as the central controller, with proper electrical isolation, level shifting, and environmental protection for reliable outdoor operation.

Edit by: Bakuho Goto
