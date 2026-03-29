## Automation and Monitoring System

This project includes an automation subsystem based on an **ESP32** for temperature control and real-time process monitoring of the biodigester.

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

The system performs three main tasks:

1. Read sensors periodically
2. Control the heating system
3. Send data through Wi-Fi to the monitoring website


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
- **DS18B20 temperature sensor** for biodigester temperature
- **DS18B20 temperature sensor** for hot water tank temperature
- **Analog pH sensor module** for slurry pH measurement

### Actuation
- **SSR (Solid State Relay)** or relay module for switching the heating system
- Optional pump control relay for circulation management

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

- Automatic pump synchronization with heater operation
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

The automation subsystem is designed to turn the biodigester into a **monitored and temperature-controlled system**, using an **ESP32** as the central controller.
