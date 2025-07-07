#include "LinPotDendro_Driver.h"



#define DENDRO_TRIM_SIZE DENDRO_SAMPLE_SIZE/2

float voltage = 0, data;
byte highbyte, lowbyte, configRegister;
float delta_x;
float xPos = 0;
float microns = 0;
float uncalibrated = 0;

// MICROMETER CALIBRATION CONSTANTS
// double A = M_A;
// double B = M_B;
// double C = M_C;
// double D = M_D;
// double E = M_E;
// double F = M_F;


long lastMillisDendro = 0;



//static const char *TAG = "example";

Adafruit_MCP3421 mcp;

// double rig_trendline(double x) {
//   return R_A*pow(x,5) + R_B*pow(x,4) + R_C*pow(x,3) + R_D*pow(x,2) + R_E*x + R_F;
// }


void dendroSetup(){

  int dendroState = 0;
  if(wilo.i2cDeviceType==I2C_MUX || wilo.dendroconnected){
  for (int port = 0; port < 8; port++){
    if (i2cDevice[port].sensorType == DENDROMETER)
    {
      dendroState=1;
      Serial.print("Dendrometer exists on port: ");
      Serial.println(port);
      I2C_Mux_SelectPort(port);
      setDendroParameters();
      return;
    }
  }
  if(wilo.dendroconnected){
    dendroState=1;
    Serial.println("Dendrometer connected on default port, setting ADC parameters: ");
    setDendroParameters();
  }} 
if (!dendroState){
    Serial.println("No dendrometer connected, setting dendro done");
    DENDRO_DONE=1;
}
  }


void setDendroParameters(){

  digitalWrite(I2C_PORT_EN_PIN, HIGH); // turn the DENDRO on (HIGH is the voltage level)
 Serial.println("Dendro En Done");

  delay(2333);

  // Initialize I2C communication
  Wire.begin(I2C_SDA, I2C_SCL, 400000);

  // Check if MCP3421 chip is found
  if (!mcp.begin(0x68, &Wire))
  {
    Serial.println("Failed to find dendrometer");
    DENDRO_DONE = 1;
    // Wire.end();
    return;
  }
 Serial.println("Dendrometer found!");

  // Set the gain of the ADC
  mcp.setGain(GAIN_1X);
  Serial.print("Gain set to: ");
  switch (mcp.getGain())
  {
  case GAIN_1X:
    Serial.println("1X");
    break;
  case GAIN_2X:
    Serial.println("2X");
    break;
  case GAIN_4X:
    Serial.println("4X");
    break;
  case GAIN_8X:
    Serial.println("8X");
    break;
  }

  // Set the resolution of the ADC
  mcp.setResolution(RESOLUTION_18_BIT); // 240 SPS (12-bit)
  Serial.print("Resolution set to: ");
  switch (mcp.getResolution())
  {
  case RESOLUTION_12_BIT:
    Serial.println("12 bits");
    break;
  case RESOLUTION_14_BIT:
    Serial.println("14 bits");
    break;
  case RESOLUTION_16_BIT:
    Serial.println("16 bits");
    break;
  case RESOLUTION_18_BIT:
    Serial.println("18 bits");
    break;
  }

  // Set the mode of the ADC
  mcp.setMode(MODE_CONTINUOUS); // Options: MODE_CONTINUOUS, MODE_ONE_SHOT
  Serial.print("Mode set to: ");
  switch (mcp.getMode())
  {
  case MODE_CONTINUOUS:
    Serial.println("Continuous");
    break;
  case MODE_ONE_SHOT:
    Serial.println("One-shot");
    break;
  }



  Serial.println("******************************************************");
}


void Dendro_Measure() {

  //long currentMillis = millis();
  float microns = 0.0;

if(!DENDRO_DONE){

  if(wilo.i2cDeviceType == I2C_MUX) {
     for ( int port = 0; port < 8; port++) {
       if (i2cDevice[port].sensorType == DENDROMETER) {
        I2C_Mux_SelectPort(port);
        if (mcp.isReady())
      {
         Serial.println("###############################################");
        Serial.print("Dendrometer is ready on port: ");
        Serial.println(port);
        int32_t adcValue = mcp.readADC(); // Read ADC value
        Serial.print("ADC Value: ");
        i2cDevice[port].arrDendroRaw[i2cDevice[port].sampleCounterDendro] = adcValue;
        Serial.println(i2cDevice[port].arrDendroRaw[i2cDevice[port].sampleCounterDendro]);
        i2cDevice[port].sampleCounterDendro++; // Increment the sample count
        Serial.print("Sample Counter: ");
        Serial.println(i2cDevice[port].sampleCounterDendro); 
          Serial.println("******************************************************");
      }

      if (i2cDevice[port].sampleCounterDendro == DENDRO_SAMPLE_SIZE) {
        
        DENDRO_DONE = 1; // Set the flag to indicate measurement is done
        Serial.println("Dendrometer measurement done, processing data...");
        bubbleSort(i2cDevice[port].arrDendroRaw, DENDRO_SAMPLE_SIZE);

        // Calculate the trimmed mean of the ADC values
        i2cDevice[port].DendroRawTrimmed = trimmedMean(i2cDevice[port].arrDendroRaw, DENDRO_SAMPLE_SIZE, DENDRO_TRIM_SIZE);
        Serial.print("Raw, trimmed mean of ADC values: ");
        Serial.println(i2cDevice[port].DendroRawTrimmed);

        microns = 10000.0 - float(i2cDevice[port].DendroRawTrimmed) * 10000.0 / 131071.0;

        if (microns == 10000.0) {
          microns = 9999.99;
        }

        Serial.print("Dendrometer Measurement on port ");
        Serial.print(port);
        Serial.print(": ");
        Serial.println(microns);
        
        i2cDevice[port].microns = microns;
        
        Serial.println("******************************************************");
        
        i2cDevice[port].sampleCounterDendro = 0; // Reset sample counter for next measurement
      }
 }
}
} else{
   if (mcp.isReady())
      {
         Serial.println("###############################################");
        Serial.print("Dendrometer is ready on default port: ");
      
        int32_t adcValue = mcp.readADC(); // Read ADC value
        Serial.print("ADC Value: ");
        wilo.arrDendroRaw[wilo.arrSampleCounterDendro] = adcValue;
        Serial.println(wilo.arrDendroRaw[wilo.arrSampleCounterDendro]);
        wilo.arrSampleCounterDendro++; // Increment the sample count
        Serial.print("Sample Counter: ");
        Serial.println(wilo.arrSampleCounterDendro); 
          Serial.println("******************************************************");
      }

      if (wilo.arrSampleCounterDendro == DENDRO_SAMPLE_SIZE) {
        
        DENDRO_DONE = 1; // Set the flag to indicate measurement is done
        Serial.println("Dendrometer measurement done, processing data...");
        bubbleSort(wilo.arrDendroRaw, DENDRO_SAMPLE_SIZE);

        // Calculate the trimmed mean of the ADC values
        wilo.DendroRawTrimmed = trimmedMean(wilo.arrDendroRaw, DENDRO_SAMPLE_SIZE, DENDRO_TRIM_SIZE);
        Serial.print("Raw, trimmed mean of ADC values: ");
        Serial.println(wilo.DendroRawTrimmed);

        microns = 10000.0 - float(wilo.DendroRawTrimmed) * 10000.0 / 131071.0;

        if (microns == 10000.0) {
          microns = 9999.99;
        }

        Serial.print("Dendrometer Measurement on default port ");
        Serial.print(": ");
        Serial.println(microns);
        
        wilo.Dendro = microns;
        
        Serial.println("******************************************************");
        
        wilo.arrSampleCounterDendro = 0; // Reset sample counter for next measurement
      }
}
}
}

// void Dendro_Measure() {

//   long currentMillis = millis();

// if(!DENDRO_DONE){

//    if(wilo.i2cMuxConnected){

//      for ( int port = 0; port < 8; port++) {
//        if (i2cDevice[port].sensorType == DENDROMETER) {
//         I2C_Mux_SelectPort(port);
//         if (mcp.isReady())
//       {
//          Serial.println("###############################################");
//         Serial.print("Dendrometer is ready on port: ");
//         Serial.println(port);
//         int32_t adcValue = mcp.readADC(); // Read ADC value
//         Serial.print("ADC Value: ");
//         i2cDevice[port].arrDendroRaw[i2cDevice[port].sampleCounterDendro] = adcValue;
//         Serial.println(i2cDevice[port].arrDendroRaw[i2cDevice[port].sampleCounterDendro]);
//         i2cDevice[port].sampleCounterDendro++; // Increment the sample count
//         Serial.print("Sample Counter: ");
//         Serial.println(i2cDevice[port].sampleCounterDendro); 
//           Serial.println("******************************************************");
//       }

//       if (i2cDevice[port].sampleCounterDendro == DENDRO_SAMPLE_SIZE) {
        
//         float dendroMicrons = 0.0;
//         DENDRO_DONE = 1; // Set the flag to indicate measurement is done
//         Serial.println("Dendrometer measurement done, processing data...");
//         bubbleSort(i2cDevice[port].arrDendroRaw, DENDRO_SAMPLE_SIZE);

//         // Calculate the trimmed mean of the ADC values
//         i2cDevice[port].DendroRawTrimmed = trimmedMean(i2cDevice[port].arrDendroRaw, DENDRO_SAMPLE_SIZE, DENDRO_TRIM_SIZE);
//         Serial.print("Raw, trimmed mean of ADC values: ");
//         Serial.println(i2cDevice[port].DendroRawTrimmed);

//         dendroMicrons = 10000.0 - float(i2cDevice[port].DendroRawTrimmed) * 10000.0 / 131071.0;

//         if (dendroMicrons == 10000.0) {
//           dendroMicrons = 9999.99;
//         }

//         Serial.print("Dendrometer Measurement on port ");
//         Serial.print(port);
//         Serial.print(": ");
//         Serial.println(dendroMicrons);

//         }
    
       
//       }
//  }
// }else if(wilo.dendroconnected){
//      if (mcp.isReady()&&wilo.arrSampleCounterDendro <= DENDRO_SAMPLE_SIZE)
//       {
//          Serial.println("###############################################");
//         Serial.print("Dendrometer is ready on port: ");
//         Serial.println("default");
//         int32_t adcValue = mcp.readADC(); // Read ADC value
//         Serial.print("ADC Value: ");
//         wilo.arrDendroRaw[wilo.arrSampleCounterDendro] = adcValue;
//         Serial.println(wilo.arrDendroRaw[wilo.arrSampleCounterDendro]);
//         wilo.arrSampleCounterDendro++; // Increment the sample count
//         Serial.print("Sample Counter: ");
//         Serial.println(wilo.arrSampleCounterDendro); 
//           Serial.println("******************************************************");
//       }

//       if (wilo.arrSampleCounterDendro == DENDRO_SAMPLE_SIZE) {
        
//         float dendroMicrons = 0.0;
//         DENDRO_DONE = 1; // Set the flag to indicate measurement is done
//         Serial.println("Dendrometer measurement done, processing data...");
//         bubbleSort(wilo.arrDendroRaw, DENDRO_SAMPLE_SIZE);

//         // Calculate the trimmed mean of the ADC values
//         wilo.DendroRawTrimmed = trimmedMean(wilo.arrDendroRaw, DENDRO_SAMPLE_SIZE, DENDRO_TRIM_SIZE);
//         Serial.print("Raw, trimmed mean of ADC values: ");
//         Serial.println(wilo.DendroRawTrimmed);

//         dendroMicrons = 10000.0 - float(wilo.DendroRawTrimmed) * 10000.0 / 131071.0;

//         if (dendroMicrons == 10000.0) {
//           dendroMicrons = 9999.99;
//         }

//         Serial.print("Dendrometer Measurement on port ");
//         Serial.print("Default");
//         Serial.print(": ");
//         Serial.println(dendroMicrons);

//         }
//         Serial.println("******************************************************");
      
//       }else{
//         Serial.println("Dendrometer not connected, skipping measurement.");
//         DENDRO_DONE = 1; // Set the flag to indicate measurement is done
//       }
  
// }
// }


