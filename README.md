# Smart Waste Segregation System

An IoT-based Smart Waste Segregation System built using ESP32, an ultrasonic sensor, a digital soil moisture sensor, and an SG90 servo motor.

The system automatically detects waste, classifies it as dry or wet, and directs it into the appropriate bin. It also provides a live Wi-Fi dashboard for monitoring the system from a laptop.

## Features

- Automatic waste detection using an ultrasonic sensor.
- Dry and wet waste classification using a soil moisture sensor.
- Automatic waste sorting using an SG90 servo motor.
- ESP32-based Wi-Fi connectivity.
- Live web dashboard accessible from a laptop.
- Real-time ultrasonic distance monitoring in centimetres.
- Dry and wet waste counters.
- Servo position monitoring.
- Standalone Wi-Fi access point; no internet connection required.

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 Dev Module | Main controller and Wi-Fi |
| HC-SR04 Ultrasonic Sensor | Waste detection and distance measurement |
| Digital Soil Moisture Sensor | Dry/wet classification |
| SG90 Servo Motor | Waste sorting mechanism |
| Breadboard | Circuit connections |
| Jumper Wires | Electrical connections |
| Cardboard Box | Project enclosure |

## Pin Connections

| Component | ESP32 GPIO |
|---|---|
| Ultrasonic TRIG | GPIO 32 |
| Ultrasonic ECHO | GPIO 33 |
| Servo Signal | GPIO 27 |
| Soil Sensor DO | GPIO 34 |

## Working Principle

1. The ultrasonic sensor detects when waste is placed within the configured detection distance.
2. The soil moisture sensor determines whether the waste is dry or wet.
3. The ESP32 processes the sensor output.
4. The SG90 servo moves the waste towards the appropriate bin.
5. The servo returns to its neutral position after sorting.
6. The ESP32 sends monitoring data to the laptop dashboard through Wi-Fi.

## Wi-Fi Dashboard

The ESP32 creates its own Wi-Fi access point. A laptop can connect directly to it and open the dashboard in a web browser.

Dashboard address: `http://192.168.4.1`

The dashboard displays:
- Ultrasonic distance.
- Waste detection status.
- Latest dry/wet classification.
- Servo position.
- Total dry waste count.
- Total wet waste count.

## Software and Technologies

- Arduino IDE
- Embedded C++ / Arduino
- ESP32 Wi-Fi
- ESP Async WebServer
- AsyncTCP
- HTML
- CSS
- JavaScript

## Project Status

Completed and tested prototype.

## Future Improvements

- Cloud-based monitoring.
- Mobile application integration.
- Waste collection level monitoring.
- Historical sorting analytics.

## Author

**Shreyas Yadav**

GitHub: [shreyasyadav0505-hub](https://github.com/shreyasyadav0505-hub)

---

*Developed as an IoT-based smart waste management project.*