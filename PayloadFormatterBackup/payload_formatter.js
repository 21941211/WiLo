function Decoder(b, port) {
  var values = [];
  var j = 0;
  var i = 0;
  var payloadLength = b.length; // Determine actual payload size

  // Ensure we don't exceed the bounds while processing 3-byte groups
  while (i + 2 < payloadLength) {
    values[j] = b[i] * 100 + b[i + 1] + b[i + 2] / 100.0;
    j++;
    i += 3;
  }

  // Base object
  var decoded = {
    payloadLength: payloadLength,
    Dendro: values[0], // Dendrometer (um)
    AirTemp: values[1], // Air Temperature (C)
    AirHumidity: values[2], // Air Humidity (%RH)
    SoilTemp: values[3], // Soil Temperature (C)
    SoilMoistureOrPyranometer: values[4], // Soil Moisture (SMV) or Pyranometer measurement (V)
    BatteryLevel: values[5], // Battery level (%)
    T1Before: values[6], // T1 (upstream) Avg Temp BEFORE heat pulse (C)
    T2Before: values[7], // T2 (downstream) Avg Temp BEFORE heat pulse (C)
    T1During: values[8], // T1 (upstream) Avg Temp DURING heat pulse (C)
    T2During: values[9], // T2 (downstream) Avg Temp DURING heat pulse (C)
    T1After: values[10], // T1 (upstream) Avg Temp AFTER heat pulse (C)
    T2After: values[11], // T2 (downstream) Avg Temp AFTER heat pulse (C)
    HPVMarshall: values[12], // HPV using Marshall formula
    BootCount: values[13]*100, //Boot count
  };

  // Add extra fields if payload length is greater than 42
  if (payloadLength > 42) {// Add extra fields if payload length is greater than 42, the default payload length

      if (payloadLength == 51){ // CS655 payload
          Object.assign(decoded, {
              CS655SoilMoisture1 : values[14],
              CS655SoilMoisture2 : values[15],
              CS655Temperature : values[16],
          });
        } else{
  
    Object.assign(decoded, { // Add extra fields for Sentek Series III
      SDISoilMoisture1: values[14],
      SDISoilMoisture2: values[15],
      SDISoilMoisture3: values[16],
      SDISoilMoisture4: values[17],
      SDISoilMoisture5: values[18],
      SDISoilMoisture6: values[19]
    });


    

    // Add even more fields if payload length is greater than 78
    if (payloadLength > 78) { //Sentek Series III 90cm
      Object.assign(decoded, {
        SDISoilMoisture7: values[20],
        SDISoilMoisture8: values[21],
        SDISoilMoisture9: values[22],
        SDITemp1: values[23],
        SDITemp2: values[24],
        SDITemp3: values[25],
        SDITemp4: values[26],
        SDITemp5: values[27],
        SDITemp6: values[28],
        SDITemp7: values[29],
        SDITemp8: values[30],
        SDITemp9: values[31],
        
      });
    } else {
      Object.assign(decoded, { //Sentek Series III 60cm
        SDITemp1: values[20],
        SDITemp2: values[21],
        SDITemp3: values[22], 
        SDITemp4: values[23],
        SDITemp5: values[24],
        SDITemp6: values[25]
      });
    }
  }
}
  return decoded;
}
