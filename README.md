# Smart Environment Monitoring System (ESP32)

This project implements an IoT-based smart environment monitoring system using ESP32.

## Sensors Used

- DHT22 (Temperature & Humidity)
- LDR (Light Sensor)
- PIR Motion Sensor
- Ultrasonic Distance Sensor

## Actuators

- LED
- Buzzer
- Relay Module
- Servo Motor

## Communication

The ESP32 sends sensor data using the MQTT protocol.

MQTT Topics:

sensors/temperature  
sensors/humidity  
sensors/light  
sensors/motion  
sensors/distance  

## Simulation

The system was simulated using the Wokwi simulator.

## Features

- Real-time sensor monitoring
- MQTT data publishing
- Automatic actuator control
- IoT-based monitoring system
