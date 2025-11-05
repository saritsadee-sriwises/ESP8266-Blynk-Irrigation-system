# ESP8266-Blynk-Irrigation-system
IoT project for automatic plant watering using ESP8266, Blynk platform, and a moisture sensor. Includes Manual/Auto modes and a 10-minute safety timer.

## Features
- Auto / Manual modes (V2 / V1)
- Hysteresis (+3%) to avoid pump chattering
- 10-minute safety cutoff
- LCD 16x2 via I2C (0x27), live moisture display
- Blynk virtual pins: V0 (moisture %), V1 (Manual), V2 (Auto), V3 (Threshold)

## Hardware
![image alt](https://github.com/saritsadee-sriwises/ESP8266-Blynk-Irrigation-system/blob/main/hardware.png?raw=true)
- ESP8266 (NodeMCU)
- Soil moisture sensor → **A0**
- Relay (active-LOW) → **D5** (pump)
- LCD I2C (0x27): **SDA=D2**, **SCL=D1**

## Setup
1. Open `Irrigation-system.ino` in Arduino IDE and upload.
2. In Blynk app create widgets:
   - Gauge **V0**
   - Button **V1** (Manual)
   - Switch **V2** (Auto)
   - Slider **V3** (0–100, threshold)
3. Fill your **Blynk token**, **Wi-Fi SSID/PASS**.

## Safety
Pump turns off automatically after 10 minutes of continuous run and resets V1/V2 to OFF.

## License
MIT
