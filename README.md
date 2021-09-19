# Environment
- Arduino: v1.0.6 (ESP-IDF v3.3.5-1-g85c43024c)
- Hardware: Lolin 32 Lite [4MB Flash, 320KB RAM] / Heltec Wireless Stick v1.2 [8MB Flash, 320KB RAM]
- IDE: PlatformIO

# Description

This example uses esp-idf's built-in mqtt client connecting to a MQTT broker over secure websockets. Serial Bluetooth is also enabled at the same time.
The example shows that the mqtt client is unable to allocate enough memory to start the tls client. I have also tried with BLE but I get the same results. Memory usage is output at various stages to illustrate the issue.