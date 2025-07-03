function decodeUplink(input) {
  const b = input.bytes;
  const port = input.fPort;
  const payloadLength = b.length;
  const decoded = {};
  const warnings = [];
  let i = 0;

  const WiLoConfig = b[payloadLength - 3];
  const SensorsTypeOnPorts = b[payloadLength - 2];
  const PortsOfMuxConnected = b[payloadLength - 1];

  function isBitSet(value, bitPosition) {
    return (value & (1 << bitPosition)) !== 0;
  }

  function isSDI12Connected(config) {
    const mask = 0b00000110;
    const DD_60CM = 0b00000010;
    const DD_90CM = 0b00000100;
    const CS655 = 0b00000110;
    const maskedbyte = config & mask;

    switch (maskedbyte) {
      case DD_60CM: warnings.push("Sentek 60cm D&D connected"); return 1;
      case DD_90CM: warnings.push("Sentek 90cm D&D connected"); return 2;
      case CS655:  warnings.push("CS655 connected"); return 3;
      default:     warnings.push("No SDI-12 connected"); return 0;
    }
  }

  const SDI12State = isSDI12Connected(WiLoConfig);
  const muxConnected = isBitSet(WiLoConfig, 0);
  if (muxConnected) warnings.push("MUX is connected");

  // === Fixed fields ===
  decoded.Dendro = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.AirTemp = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.AirHumidity = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.SoilTemp = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.SoilMoistureOrPyranometer = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.BatteryLevel = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.T1Before = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.T2Before = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.T1During = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.T2During = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.T1After = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.T2After = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.BootCount = b[i] * 100 + b[i + 1] + b[i + 2] / 100; i += 3;
  decoded.BootCount = decoded.BootCount*100; //convert back to an integer ranging between 1 and 999999

  // === MUX decoding ===
  if (muxConnected) {
    for (let port = 0; port < 8; port++) {
      const portLabel = `P${port}`;
      if (isBitSet(PortsOfMuxConnected, port)) {
        if (isBitSet(SensorsTypeOnPorts, port)) {
          if (i + 2 < payloadLength - 3) {
            decoded[`${portLabel}_Dendro`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
            i += 3;
          }
        } else {
          const labels = ["T1B", "T2B", "T1D", "T2D", "T1A", "T2A"];
          for (const label of labels) {
            if (i + 2 < payloadLength - 3) {
              decoded[`${portLabel}_${label}`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
              i += 3;
            }
          }
        }
      }
    }
  }

  // === SDI-12 decoding ===
  switch (SDI12State) {
    case 1: // 60cm Sentek
      for (let ch = 1; ch <= 6; ch++) {
        if (i + 2 < payloadLength - 3) {
          decoded[`SDI12_Moisture_${ch}`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
          i += 3;
        }
      }
      for (let ch = 1; ch <= 6; ch++) {
        if (i + 2 < payloadLength - 3) {
          decoded[`SDI12_Temp_${ch}`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
          i += 3;
        }
      }
      break;
    case 2: // 90cm Sentek
      for (let ch = 1; ch <= 9; ch++) {
        if (i + 2 < payloadLength - 3) {
          decoded[`SDI12_Moisture_${ch}`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
          i += 3;
        }
      }
      for (let ch = 1; ch <= 9; ch++) {
        if (i + 2 < payloadLength - 3) {
          decoded[`SDI12_Temp_${ch}`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
          i += 3;
        }
      }
      break;
    case 3: // CS655
      for (let ch = 1; ch <= 2; ch++) {
        if (i + 2 < payloadLength - 3) {
          decoded[`SDI12_Moisture_${ch}`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
          i += 3;
        }
      }
      if (i + 2 < payloadLength - 3) {
        decoded[`SDI12_Temp_1`] = b[i] * 100 + b[i + 1] + b[i + 2] / 100;
        i += 3;
      }
      break;
  }

  return {
    data: decoded,
    warnings: warnings,
    errors: []
  };
}
