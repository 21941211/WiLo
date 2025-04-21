# Payload Formatter Backup

This folder contains a backup of the custom payload formatter used to decode sensor data in a The Things Network (TTN) application. The file `payload_formatter.js` contains a JavaScript function used to parse binary payloads received from LoRaWAN devices into structured, human-readable values.

## Overview

The decoder processes 3-byte sensor readings sent in a single uplink message. It supports multiple configurations depending on the connected sensors and payload length:

- Core environmental readings: dendrometer, air/soil temperature, humidity, battery, heat pulse velocity, etc.
- Support for up to three SDI-12 sensors:
  - **Sentek Drill & Drop 60cm (6 segments)**
  - **Sentek Drill & Drop 90cm (9 segments)**
  - **CS655 soil moisture probe**

Values are decoded from the byte array, converted to engineering units, and returned in a structured JSON format.

---

## Files

- `payload_formatter.js` – Contains the JavaScript decoder function (`Decoder(b, port)`).
- `README.md` – This file, with usage instructions and context.

---

## How to Import into a TTN Application

To use this payload formatter in your TTN application:

### 1. Open the TTN Console
- Go to [TTN Console](https://console.thethingsnetwork.org/) and open your application.

### 2. Navigate to the Payload Formatters
- In the application view, go to **Payload formatters**.
- Select the **Uplink** tab.

### 3. Set Formatter Type
- Choose **Custom JavaScript**.

### 4. Paste the Code
- Open `payload_formatter.js` in your text editor.
- Copy the entire contents.
- Paste it into the custom JavaScript box in the TTN console.

### 5. Save Changes
- Click **Save changes**.

Once saved, TTN will automatically apply this decoder to every uplink payload received from devices under this application.

---

## Notes

- This formatter assumes each sensor reading is encoded using **3 bytes**: high byte, low byte, and fractional byte.
- The number of decoded fields depends on the payload length and the type of SDI-12 sensor connected.
- Make sure your devices and firmware send payloads in the expected format, or update this script accordingly.

For questions or to adapt the decoder to additional sensor types, consult your firmware documentation or contact your development team.

---
