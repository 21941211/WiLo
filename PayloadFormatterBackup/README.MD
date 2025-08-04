# Payload Formatter for WiLo V2

This folder contains a backup of the custom payload formatter used to decode uplink messages from WiLo V2 LoRaWAN devices in The Things Network (TTN). The JavaScript decoder (`payloadFormatter.js`) transforms raw binary payloads into structured, human-readable sensor data.

---

## ⚙️ Overview

This decoder handles dynamic payloads from WiLo V2 based on attached sensors. It supports:

- **Core onboard sensors**: dendrometer, air temperature & humidity, soil temperature, soil moisture or pyranometer, and battery voltage.
- **Sapflow sensors**: heat pulse velocity data (before, during, after) from dual probes.
- **MUX expansion support** for up to 8 external I²C devices.
- **SDI-12 sensors**, including:
  - Sentek Drill & Drop 60 cm (6 segments)
  - Sentek Drill & Drop 90 cm (9 segments)
  - Campbell Scientific CS655

Each 3-byte sensor reading includes:  
`[High Byte][Low Byte][Fractional Byte]` → converted to `float = HB×100 + LB + FB/100`

---

## 🧠 Payload Format Logic

At the end of each payload, 3 configuration bytes define the payload structure:

| Byte | Purpose |
|------|---------|
| `WiLoConfig` | Flags for MUX, SDI-12 sensor type |
| `SensorsTypeOnPorts` | Indicates dendrometer (1) or sapflow (0) per MUX port |
| `PortsOfMuxConnected` | Bitmask for which MUX ports are active |

---

## 📥 Decoded Fields

### Fixed Fields (Always Present)

| Field | Description |
|-------|-------------|
| `Dendro` | Internal dendrometer reading |
| `AirTemp` | Air temperature (°C) |
| `AirHumidity` | Relative humidity (%) |
| `SoilTemp` | Soil temperature (°C) |
| `SoilMoistureOrPyranometer` | Depending on connected sensor |
| `BatteryLevel` | Battery voltage (V) |
| `T1Before`, `T2Before` | Pre-heat sapflow temps |
| `T1During`, `T2During` | Heating period temps |
| `T1After`, `T2After` | Post-heat temps |
| `BootCount` | Device restart count (integer) |

### MUX-Connected Devices

If a MUX is detected (`WiLoConfig` bit 0), up to 8 ports (`P0`–`P7`) can report:

- **Dendrometer**: `P{n}_Dendro`
- **Sapflow** (T1/T2 at each stage):  
  `P{n}_T1B`, `P{n}_T2B`, `P{n}_T1D`, `P{n}_T2D`, `P{n}_T1A`, `P{n}_T2A`

### SDI-12 Sensors

Determined from `WiLoConfig` bits 1 and 2:

- `0b10` → Sentek Drill & Drop 60 cm  
  - Fields: `SDI12_Moisture_1`–`6`, `SDI12_Temp_1`–`6`
- `0b100` → Sentek Drill & Drop 90 cm  
  - Fields: `SDI12_Moisture_1`–`9`, `SDI12_Temp_1`–`9`
- `0b110` → CS655  
  - Fields: `SDI12_Moisture_1`–`2`, `SDI12_Temp_1`

Any unrecognized value → “No SDI-12 connected” warning.

---

## 🛠 Files

| File | Purpose |
|------|---------|
| `payload_formatter.js` | The custom decoder function (`decodeUplink(input)`) |
| `README.md` | This documentation |

---

## 🌐 How to Import into TTN Console

1. Log in to [TTN Console](https://console.thethingsnetwork.org/)
2. Go to your **application**
3. Navigate to **Payload formatters → Uplink**
4. Choose **Custom JavaScript**
5. Paste contents of `payload_formatter.js`
6. Click **Save changes**

TTN will now automatically decode incoming uplinks using this formatter.

---

## 🔍 Notes

- All sensor readings are 3-byte floats.
- Decoder handles **variable payload lengths** based on actual sensor configuration.
- MUX-connected devices are supported dynamically using port bitmasks.
- Boot count is scaled back to integer (range 1–999,999).

For questions, support, or customization for additional sensors, refer to firmware documentation or contact the development team.

---
