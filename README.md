# Smart-Compost-Monitoring-and-Automated-Stirring-System
Designed and implemented an intelligent IoT-based compost monitoring system using Arduino, enabling real-time measurement and automated control of compost health parameters.

Key Contributions:

Built a multi-sensor system integrating DHT11 (temperature & humidity) and an analog soil-moisture sensor, with dynamic calibration for accurate compost condition tracking.

Developed an intuitive 16×2 I²C LCD interface for live display of temperature, humidity, soil moisture %, and system status.

Programmed a dual-servo mechanism:

Stirring servo for automated compost aeration.

Valve servo for controlled water dispensing based on soil-moisture thresholds.

Designed a feedback-control loop with:

Moisture-based automatic decision-making

6-hour interval lock to prevent over-stirring

Coordinated sequencing of valve → stir → valve-close cycles

Implemented robust firmware using millis(), sensor averaging, motor control, and LCD state management to ensure stable, non-blocking operation.

Optimized wiring and power management to safely operate multiple servos and sensors simultaneously.

Demonstrated strong skills in embedded programming, sensor integration, control systems, and hardware troubleshooting.

Tech Stack:
Arduino UNO, C/C++, DHT11, Resistive Soil Moisture Sensor, I²C LCD, SG90 Servos, Embedded Systems, Sensors, Actuators, PWM, Control Logic, Real-Time Data Processing.
