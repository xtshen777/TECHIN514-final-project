# Desk Emotion Indicator

## 1. Project overview

### Desk Emotion Indicator 
Cat-Ear Headband–based PPG Emotion Sensing System

### What it does  
The Desk Emotion Indicator is a two device system that uses a cat ear headband to measure heart related signals at the temples and a desk display with a mechanical gauge needle to visualize emotional trends from relaxed to stressed. The system is designed for desk based study or work, giving a calm and glanceable reflection of emotional state rather than exact numbers.

### General Sketch  
![Personal Safety Detector & Alarm Sketch](images/general_sketch.jpg)
The sensing device is a cat shaped headband with two wide side sections that gently cover and touch the temple areas. A PPG sensor module is embedded inside each wide section so that the optical window rests against the skin, with soft foam pads and light blocking rings for comfort and signal quality. The cat ears on top of the headband are slightly thickened and used to hide the PCB, ESP32 based microcontroller and battery, as well as to route and organize the sensor wiring.

The display device is a small desk unit that looks like a vintage analog meter. It has a mechanical gauge needle driven by a stepper motor to show a continuous emotional state from relaxed to tense, a row of LEDs below the gauge as a secondary indication and a single reset or acknowledge button. The desk unit receives processed heart rate features wirelessly from the headband and updates the gauge and LEDs smoothly.

---

## 2. Sensor device (cat ear headband)

### Concept and placement  
![Sensor Device](images/sensor_sketch.jpg)
The sensor device is a cat ear headband that integrates PPG sensing modules at the temple positions. The headband sides are widened in the region that covers the temples to create space for the PPG boards and to provide a larger, more comfortable contact surface. On the inner side, soft foam pads and a small dark ring around each PPG window help maintain gentle pressure, reduce ambient light and improve signal quality. The cat ears on top of the headband are used as a functional enclosure that hides the main PCB, ESP32 module and battery, while also serving as a playful form language.

### Electronics and part numbers  
Planned parts for the sensor device include:

- Processor and wireless  
  - Seeed Studio XIAO ESP32C3 (ESP32 C3 based MCU with BLE)

- Optical heart sensor  
  - MAX30102 pulse oximeter and heart rate sensor module  
    - PPG sensing for heart rate and heart rate variability features  
  - Initially one MAX30102 module at one temple for prototyping, with the option to extend to both sides later

- Power  
  - 3.7 V LiPo battery, about 500 mAh  
  - TP4056 LiPo charger module  
  - Power switch

- Mechanical and wiring  
  - Cat ear headband shell with widened temple sections  
  - Soft foam or silicone pads around each sensor window  
  - Flexible wires routed inside the headband structure  
  - Custom small PCB in one cat ear that connects MAX30102, XIAO ESP32C3, battery and charger

### How it works  
The MAX30102 module in the temple section emits light into the skin and measures the reflected light signal that changes with blood volume. The Seeed XIAO ESP32C3 reads raw PPG samples over I2C, applies basic filtering and peak detection and computes heart rate and short term heart rate variability features over a sliding time window. Instead of sending raw signals, the headband computes compact emotional features such as smoothed heart rate and an HRV based stress score and periodically transmits these features via BLE to the desk display device. The headband is designed for stable contact during desk work, avoiding hand and wrist motion artifacts.

---

## 3. Display device (desk gauge)

### Concept and placement  
![Display Device](images/display_sketch.jpg)
The display device is a compact desk unit that turns the incoming physiological features into a physical gauge and subtle light feedback. The main element is a mechanical needle gauge that moves smoothly between relaxed and stressed states. Below the gauge, a horizontal row of LEDs provides a secondary visualization that lights up more segments as the user becomes more tense. A single tactile button on the front allows the user to reset or acknowledge the current state or recalibrate the baseline.

### Electronics and part numbers  
Planned parts for the display device include:

- Processor and wireless  
  - Seeed Studio XIAO ESP32C3 (second board, acting as BLE central and display controller)

- Mechanical gauge  
  - 28BYJ 48 stepper motor  
  - ULN2003 stepper motor driver board

- Visual indicators  
  - LEDs, green, yellow and red or several identical color LEDs  
  - Current limiting resistors for each LED (for example 220 Ω to 330 Ω)

- User input  
  - One momentary tactile push button for reset or acknowledge

- Power  
  - 3.7 V LiPo battery, about 1000 mAh  
  - TP4056 LiPo charger module  
  - Power switch

- Mechanical  
  - Custom enclosure designed to sit on a desk and support the gauge dial  
  - Custom PCB that connects XIAO ESP32C3, ULN2003, LEDs, button and power components

### How it works  
The desk device acts as a BLE central and listens for periodic data packets from the headband sensor device. When it receives new heart rate and HRV based features, the XIAO ESP32C3 maps the emotional state score to a target gauge angle and LED pattern. The microcontroller drives the ULN2003 board to step the 28BYJ 48 motor to the new angle in a smooth way to avoid sudden jumps. The LEDs under the gauge are updated to indicate low, medium or high stress zones. The reset button allows the user to set a new baseline or clear a previously high state. The display is intentionally quiet and uses only motion and soft light to keep the feedback calm and ambient.

---

## 4. System communication and diagrams

### 4.1 High level system diagram
![Display Device](images/system_diagram.png)
- User sitting at a desk wearing the cat ear headband  
- The cat ear headband labeled as "Sensor device" with temple based PPG sensing  
- The desk gauge unit labeled as "Display device" with needle, LEDs and button  
- A BLE wireless link between headband and desk gauge  

### 4.2 Detailed data flow diagram (figure 2)
![Display Device](images/detailed_diagram.png) 

Sensor device pipeline (headband):

- MAX30102 PPG module at temple  
  - Raw optical signal samples  
- XIAO ESP32C3 sensing firmware  
  - I2C data acquisition  
  - Filtering and baseline removal  
  - Peak detection for heart beats  
  - Heart rate and HRV feature computation  
  - Emotional trend score calculation  
- BLE packet  
  - Contains heart rate, HRV features and an emotional score  

Display device pipeline (desk gauge):

- BLE central on desk XIAO ESP32C3  
  - Receives packets from the headband  
- Mapping logic  
  - Map emotional score to gauge angle range  
  - Map emotional score to LED band states  
- Stepper control  
  - XIAO ESP32C3 generates step signals  
  - ULN2003 driver powers 28BYJ 48 motor  
  - Gauge needle moves to new angle  
- LED and user interface  
  - LEDs show low, medium or high emotional bands  
  - User can press the reset button to set a new baseline or acknowledge a high state  


---

## 5. Datasheets

All component datasheets used in this project are available in the `datasheets/` folder:

- [Seeed XIAO ESP32C3](datasheets/Seeed_XIAO_ESP32C3.pdf)
- [MAX30102 PPG Sensor](datasheets/MAX30102_sensor.pdf)
- [28BYJ-48 Stepper Motor](datasheets/28BYJ48_stepper_motor.pdf)
- [ULN2003 Driver](datasheets/ULN2003_driver.pdf)
- [TP4056 Charger](datasheets/TP4056_charger.pdf)
- [LiPo Battery](datasheets/LiPo_battery.pdf)
- [Tactile Switch](datasheets/tactile_switch.pdf)
- [NeoPixel Digital RGB LED Strip](datasheets/NeoPixel_LED.pdf)



