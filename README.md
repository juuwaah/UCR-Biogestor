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

- **pH of the slurry**  
  To monitor fermentation conditions and process stability

---

## General System Description

This automation system is based on an ESP32 microcontroller connected to the sensors and to the heating control stage.

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

## Hardware Considered

### Microcontroller
- **ESP32**

### Sensors
- **MAX6675 thermocouple module** for hot water tank temperature
- **DS18B20 temperature sensor** for biodigester slurry temperature and pump temperature management
- **Analog pH sensor module** for slurry pH measurement

### Display
- **20x4 LCD 2004A Module (5V Blue Screen)** with I2C interface for local real-time display of tank temperature and heating status

### Actuation
- **SSR (Solid State Relay)** for switching the water heater
- **SSR (Solid State Relay)** for circulation pump control, with temperature management via DS18B20

---

## Wiring Diagram

### GPIO Pin Assignment

| Component | Signal | ESP32 GPIO |
|-----------|--------|------------|
| MAX6675 | SCK | GPIO18 |
| MAX6675 | CS | GPIO5 |
| MAX6675 | SO (MISO) | GPIO19 |
| DS18B20 (x2) | DATA | GPIO4 |
| LCD 2004A (I2C) | SDA | GPIO21 |
| LCD 2004A (I2C) | SCL | GPIO22 |
| pH Sensor | Analog Out | GPIO34 (ADC1) |
| SSR - Heater | Control | GPIO25 |
| SSR - Pump | Control | GPIO26 |

> **Note:** GPIO34 is used for the pH sensor because ADC2 pins are unavailable when Wi-Fi is active. Only ADC1 pins (GPIO 32-39) can be used for analog readings with Wi-Fi enabled.

> **Note:** Multiple DS18B20 sensors (biodigester + pump) share the same OneWire data line (GPIO4). Each sensor has a unique address and is identified automatically.

### Power Distribution

| Component | Voltage | Source |
|-----------|---------|--------|
| LCD 2004A | 5V | ESP32 VIN (5V) |
| MAX6675 | 5V | ESP32 VIN (5V) |
| pH Sensor Module | 5V | ESP32 VIN (5V) |
| DS18B20 (x2) | 3.3V | ESP32 3.3V |

All GND pins are connected to a common ground rail.

### Wiring Notes

```
ESP32 5V (VIN) ─── Breadboard + Rail ──┬── LCD VCC
                                       ├── MAX6675 VCC
                                       └── pH Module VCC

ESP32 3.3V ──────── DS18B20 VCC (Red wire, both sensors)

ESP32 GND ──────── Breadboard - Rail ──┬── LCD GND
                                       ├── MAX6675 GND
                                       ├── DS18B20 GND (Black wire, both sensors)
                                       └── pH Module GND

DS18B20 DATA (Yellow wire, both sensors) ── GPIO4
                                            └── 4.7kΩ pull-up resistor to 3.3V
```

> **Important:** A 4.7kΩ pull-up resistor is required between the DS18B20 data line and 3.3V for reliable OneWire communication.

---

## Local LCD Display

A 20x4 LCD (2004A, 5V Blue Screen) is connected to the ESP32 via I2C to provide on-site monitoring without requiring network access.

### Information displayed on the LCD
- Line 1: Biodigester slurry temperature
- Line 2: Hot water tank temperature
- Line 3: Heating system status (ON / OFF)
- Line 4: Pump status (ON / OFF)

This allows operators to check the system status directly at the installation site.

---

## Data Transmission

The ESP32 connects to a local Wi-Fi network and uploads sensor values to a custom website (tentatively considered to be a website via Railway) developed for this project.

### Data sent to the website
- Biodigester temperature
- Hot water tank temperature
- pH
- Heating system status
- Timestamp

This makes it possible to observe the system remotely in real time and keep a historical record of operation.

---

## Monitoring Website

The monitoring platform receives the data transmitted by the ESP32 and displays the process variables in real time.

### Expected functions of the website
- Live display of biodigester temperature
- Live display of hot water tank temperature
- Live display of pH
- Heater status visualization
- Historical data storage for later analysis
- Future alarm or notification system

---

## Current Scope of the Automation Subsystem

At this stage, the automation subsystem is defined to include:

- Real-time sensing of key variables
- Basic automatic temperature control
- Wi-Fi communication with the monitoring website
- Data logging for process evaluation

---

## Future Improvements

Possible future upgrades include:

- Safety shutdown in case of sensor failure
- Maximum water temperature protection
- Remote setpoint adjustment from the website
- Alarm system for abnormal pH or temperature values
- Full dashboard with historical graphs

---

## Initial Reference Logic

An initial version of the control logic was first developed in Arduino-style code using a DS18B20 sensor and SSR-based hysteresis control.  
This logic will be migrated and expanded to the ESP32 platform, adding:

- Two temperature sensors instead of one
- pH measurement
- Wi-Fi communication
- Data upload to the web platform

---

## Summary

The automation subsystem is designed to turn the biodigester into a monitored and temperature-controlled system, using an ESP32 as the central controller.
