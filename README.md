# Superfanzy 🌀

An ESP32 / ESP-IDF based external PWM fan controller designed for enterprise servers and custom homelab setups where software fan control is ignored or restricted.

---

## 💡 The Story Behind Superfanzy

Enterprise hardware like the **Nutanix NX8235** server platform can be loud af. In many homelab scenarios, server firmware, IPMI, or host OS commands ignore user attempts to adjust fan curves or throttle fan speeds, leaving chassis fans running at ear bleeding noise levels.

**Superfanzy** was created as a hardware bypass solution. By taking fan control out of the motherboard's hands and routing standard 4-pin PWM signals through an ESP32 microcontroller, Superfanzy lets you independently control each server fan over Wi-Fi using MQTT messages (compatible with Home Assistant, Node-RED, or CLI scripts).

---

## ✨ Features

- **Individual 4-Pin Fan Control**: Drive up to 4 separate PWM fans independently.
- **Industry-Standard 25 kHz PWM Output**: Uses ESP32's LEDC peripheral configured to standard 25 kHz PWM frequency with 10-bit resolution (duty cycle: `0` to `1023`).
- **MQTT Remote Interface**: Set duty cycles in real-time via simple MQTT publishing commands.
- **Real-Time Telemetry**: Broadcasts system status, MAC address, connection uptime, and current Wi-Fi SSID in JSON format every 5 seconds.
- **Modular ESP-IDF Architecture**: Clean separation into ESP-IDF components (`sf_wifi`, `sf_fans`, `sf_mqtt`).
- **Configurable via Kconfig**: Easily set Wi-Fi credentials and MQTT broker details using `idf.py menuconfig`.

---

## 🛠️ Hardware Pinout & Specs

| Fan ID | GPIO Pin | LEDC Channel | PWM Frequency | Resolution | Topic Endpoint |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **Fan 1** | GPIO 2 | Channel 0 | 25 kHz | 10-bit (0 - 1023) | `/fanctl/control/fan/1/PWM` |
| **Fan 2** | GPIO 4 | Channel 1 | 25 kHz | 10-bit (0 - 1023) | `/fanctl/control/fan/2/PWM` |
| **Fan 3** | GPIO 16 | Channel 2 | 25 kHz | 10-bit (0 - 1023) | `/fanctl/control/fan/3/PWM` |
| **Fan 4** | GPIO 17 | Channel 3 | 25 kHz | 10-bit (0 - 1023) | `/fanctl/control/fan/4/PWM` |

> ⚠️ **Note on Wiring**: 4-pin PC/Server fan wires usually consist of Ground (GND), 12V Power, Tachometer (RPM sense), and PWM signal input. ESP32 GPIOs output 3.3V PWM signals directly to the fan's PWM control pin. Ensure the fans share a common ground with the ESP32.

---

## 📡 MQTT Endpoints & Usage

### 1. Fan Control Topics

Set the PWM duty cycle for a fan by publishing an integer value between `0` (0% speed) and `1023` (100% speed) to the fan's control topic.

| Topic | Payload Format | Description |
| :--- | :--- | :--- |
| `/fanctl/control/fan/1/PWM` | `0`–`1023` | Sets duty cycle for Fan 1 |
| `/fanctl/control/fan/2/PWM` | `0`–`1023` | Sets duty cycle for Fan 2 |
| `/fanctl/control/fan/3/PWM` | `0`–`1023` | Sets duty cycle for Fan 3 |
| `/fanctl/control/fan/4/PWM` | `0`–`1023` | Sets duty cycle for Fan 4 |

**Examples (using `mosquitto_pub`):**

```bash
# Set Fan 1 to ~50% speed (512 / 1023)
mosquitto_pub -h <MQTT_BROKER_IP> -p 1883 -t "/fanctl/control/fan/1/PWM" -m "512"

# Set Fan 2 to full speed (1023)
mosquitto_pub -h <MQTT_BROKER_IP> -p 1883 -t "/fanctl/control/fan/2/PWM" -m "1023"

# Turn off (almost off*) Fan 3 (0)
mosquitto_pub -h <MQTT_BROKER_IP> -p 1883 -t "/fanctl/control/fan/3/PWM" -m "0"
```

---

### 2. Status & Telemetry Topic

Superfanzy periodically publishes device status and Wi-Fi network information every 5 seconds.

| Topic | Direction | Payload Format |
| :--- | :--- | :--- |
| `/fanctl/status` | Publish (ESP32 → Broker) | JSON |

**Sample Payload:**

```json
{
  "device_mac": "AA:BB:CC:DD:EE:FF",
  "status": "online",
  "uptime_s": 1284,
  "wifi_ssid": "HomeLab-Network"
}
```

**Listening to status updates:**

```bash
mosquitto_sub -h <MQTT_BROKER_IP> -p 1883 -t "/fanctl/status"
```


## 📂 Project Architecture

```
superfanzy/
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   ├── main.c           # Application entry point & Wi-Fi/MQTT initialization
│   └── idf_component.yml
└── components/
    ├── sf_fans/         # LEDC PWM hardware fan drivers & topic lookup
    │   ├── fans.c
    │   └── include/fans.h
    ├── sf_mqtt/         # MQTT client, message subscriptions, and status publisher task
    │   ├── client.c
    │   ├── Kconfig.projbuild
    │   └── include/client.h
    └── sf_wifi/         # Wi-Fi STA connection management & AP scanning
        ├── connect.c
        ├── scan.c
        ├── util.c
        └── Kconfig.projbuild
```

---

## 🚀 Building & Flashing

### Requirements
- [ESP-IDF v5.x or higher](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
- ESP32 Development Board

### 1. Configure Options
Launch the Kconfig setup menu to set your Wi-Fi credentials and MQTT broker URL:

```bash
idf.py menuconfig
```

Navigate to:
- **SF WIFI Settings**: Configure `WIFI SSID` and `WIFI Password`.
- **SF MQTT Settings**: Enable MQTT and set `SF MQTT Broker URL` (e.g., `mqtt://192.168.1.100:1883`).

### 2. Build and Flash

```bash
# Build the project
idf.py build

# Flash to device and monitor output (replace /dev/ttyUSB0 with your port)
idf.py -p /dev/ttyUSB0 flash monitor
```

---

## 📄 License

This project is open source and available under the standard MIT License.
