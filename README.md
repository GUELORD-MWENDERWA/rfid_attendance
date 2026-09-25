# RFID Attendance Terminal (ESP8266 + Arduino)

Firmware for a dual-reader RFID attendance terminal. Cards scanned at the terminal are identified, checked against the selected class, and pushed over Wi-Fi to a PHP/MySQL attendance backend with an HTTP request.

The project is split into two PlatformIO firmwares that communicate over a serial link.

## Architecture

```
 RC522 (entry) ─┐
                ├─ SPI ─> NodeMCU ESP8266 ── Wi-Fi / HTTP GET ──> getdata.php (attendance server)
 RC522 (exit)  ─┘              │
                               └── SoftwareSerial 57600 baud ──> Arduino Nano (auxiliary controller)
```

| Directory | Target | Role |
| --- | --- | --- |
| `nodemcu_presence_rfid/` | NodeMCU v2 (ESP8266) | Reads both RC522 readers, handles class selection, connects to Wi-Fi, sends card UIDs to the server, supports OTA updates |
| `arduino_presence_rfid/` | Arduino Nano (ATmega328P) | Receives and parses underscore-separated frames from the NodeMCU over serial |

## Features

- Two MFRC522 readers sharing the SPI bus (entry and exit)
- Class selection with a push button, cycling through three configured classes
- UID formatting to uppercase hexadecimal
- Event-driven Wi-Fi management (connected, got IP, disconnected handlers)
- HTTP delivery of each accepted scan, with status LED feedback
- Arduino OTA support on the ESP8266

## Hardware

- NodeMCU v2 (ESP8266)
- Arduino Nano
- 2 x MFRC522 RFID readers (SS on D2 and D3, shared RST on D1)
- Push button on D4 for class selection
- Red status LED on D0

## Configuration

Before flashing `nodemcu_presence_rfid`, edit `src/main.cpp`:

- `SSID` and `PASSWORD`: Wi-Fi credentials
- `device_token`: token that identifies this terminal to the server
- `URL`: address of the attendance endpoint (for example `http://<server>/rfidattendance/getdata.php`)
- Authorized card UIDs per class in `sellect_uid()`

Do not commit real credentials. Move them to a header excluded by `.gitignore` (for example `include/secrets.h`) before publishing a fork.

## Build

Each firmware is an independent PlatformIO project:

```bash
cd nodemcu_presence_rfid && pio run -t upload
cd ../arduino_presence_rfid && pio run -t upload
```

Dependency: `miguelbalboa/MFRC522`, resolved automatically by PlatformIO.

## Server side

The firmware expects a PHP endpoint that accepts the card UID and device token as query parameters and returns HTTP 200 on success. The server is not included in this repository.

## License

No license has been specified yet. Contact the author before reusing this code.
