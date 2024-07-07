# Smart-MediBox
![image](https://github.com/akhilaprabodha/Smart-MediBox/assets/107745538/677a3076-9149-44cb-8b4a-d6f568d02890)

![image](https://github.com/akhilaprabodha/Smart-MediBox/assets/107745538/5d9c97ac-233b-4463-9939-8ad4a4a5bc09)

# 📦 Smart Medibox

The Smart Medibox is an intelligent device crafted to support individuals in managing their medication regimen effectively. It combines automated reminders with environmental monitoring to ensure medications are stored under optimal conditions.

## Overview
The Smart Medibox is designed to help users adhere to their medication schedules while maintaining the ideal storage environment for medications. It provides timely reminders, monitors temperature and humidity, and controls light exposure to ensure medications remain effective.

### 🛠️ Key Features
- **Automated Medication Reminders:** Set alarms to remind users to take their medications on time.
- **Environmental Monitoring:** Continuously tracks temperature and humidity levels, alerting users if conditions fall outside the recommended range.
- **Light Control:** Utilizes a motorized curtain to regulate light exposure, preserving the integrity of light-sensitive medications.

### 📚 Technologies and Components
- **OLED Display:** ADAFRUIT SSD 1306 OLED Monochrome Display (128x64) for clear and accessible information display.
- **Microcontroller:** ESP32 Devkit V1, providing robust performance and connectivity.
- **Sensors:** DHT11 for temperature and humidity monitoring (can be configured for DHT22).
- **Actuators:** SG90 Micro Servo Motor for light control.
- **Additional Components:** Light Dependent Resistors (LDRs), push buttons, and 10kΩ resistors.

### 🚀 Getting Started

#### Prerequisites
- 🖥️ **Software:**
  - Git for repository management
  - PlatformIO with Arduino Framework for development and deployment
  - Node-RED for dashboard creation
  - MQTT broker for communication
- 🔩 **Hardware:**
  - ESP32 Devkit V1
  - ADAFRUIT SSD 1306 OLED Display
  - DHT11 or DHT22 sensor
  - SG90 Micro Servo Motor
  - Light Dependent Resistors (LDRs)
  - Push Buttons
  - 10kΩ Resistors

#### Installation
1. 📦 **Clone the Repository:**
```bash
git clone https://github.com/akhilaprabodha/Smart-Medibox
```

2.🖥️ **For Simulation:**
```bash
git clone -b Wokwi-Simulation https://github.com/akhilaprabodha/Smart-Medibox
```

3. ⚙️ **Set Up Development Environment:**
Ensure PlatformIO is set up with the Arduino Framework. The necessary libraries are typically installed automatically, but you can refer to the platform.ini file for manual installations.

#### Usage
1. 🔩 **Hardware Setup:**  
Connect all components according to the provided wiring diagram in the repository.

2. 📤 **Upload Code:**  
Upload the code to the ESP32 Devkit V1 using PlatformIO.

3. ⚙️ **Configure Settings:**
    - Set medication reminders.
    - Define temperature and humidity thresholds.
    - Adjust light control preferences.

4. 🌐 **Node-RED Dashboard:**
    - Import ```flows.json``` to Node-RED.
    - Configure MQTT server parameters and deploy.

### 🧩 Software Architecture
The software is organized into modular components for efficient management:

- **Hardware Abstraction Layer:** Simplifies interaction with hardware components.
- **Sensor Management:** Manages data from the DHT sensor and LDRs.
- **Alarm Management:** Handles medication reminders and notifications.
- **Time Management:** Synchronizes with an NTP server and allows time zone configuration.
- **User Interface:** Provides a menu-driven interface on the OLED display.
- **Communication Management:** Manages MQTT communication for remote data transmission and control.

### 🤝 Contributing
We welcome contributions to enhance the Smart Medibox:
- **Bug Reports:** Submit issues if you encounter any bugs.
- **Feature Requests:** Suggest new features or improvements.
