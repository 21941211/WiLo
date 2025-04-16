#include "LinPotDendro_Driver.h"

#define ads1110 0x48
#define DENDRO_SAMPLE_SIZE 10
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

int sampleCounterDendro = 0;
long lastMillisDendro = 0;


float arrDendroRaw[DENDRO_SAMPLE_SIZE] = {0};
float DendroRawTrimmed = 0;

static const char *TAG = "example";

Adafruit_MCP3421 mcp;

// double rig_trendline(double x) {
//   return R_A*pow(x,5) + R_B*pow(x,4) + R_C*pow(x,3) + R_D*pow(x,2) + R_E*x + R_F;
// }


void dendroSetup(){
  // Set up Dendrometer
  
 // Serial.println("Setting up I2C: ");
 //pinMode(SF_DENDRO_EN_PIN,OUTPUT);
digitalWrite(SF_DENDRO_EN_PIN, HIGH); // turn the DENDRO on (HIGH is the voltage level)
 Serial.println("Dendro En Done");

  delay(2333);

  

  // Initialize I2C communication
  Wire.begin(I2C_SDA, I2C_SCL, 400000);

  // Check if MCP3421 chip is found
  if (!mcp.begin(0x68, &Wire))
  {
    Serial.println("Failed to find MCP3421 chip");
    DENDRO_DONE = 1;
    // Wire.end();
    return;
    // while (1)
    // {
    //   delay(10); // Avoid a busy-wait loop
    // }
  }
 //Serial.println("MCP3421 Found!");

  // Set the gain of the ADC
  mcp.setGain(GAIN_1X);
  //Serial.print("Gain set to: ");
  switch (mcp.getGain())
  {
  case GAIN_1X:
   // Serial.println("1X");
    break;
  case GAIN_2X:
   // Serial.println("2X");
    break;
  case GAIN_4X:
   // Serial.println("4X");
    break;
  case GAIN_8X:
   // Serial.println("8X");
    break;
  }

  // Set the resolution of the ADC
  mcp.setResolution(RESOLUTION_18_BIT); // 240 SPS (12-bit)
  //Serial.print("Resolution set to: ");
  switch (mcp.getResolution())
  {
  case RESOLUTION_12_BIT:
   // Serial.println("12 bits");
    break;
  case RESOLUTION_14_BIT:
   // Serial.println("14 bits");
    break;
  case RESOLUTION_16_BIT:
   // Serial.println("16 bits");
    break;
  case RESOLUTION_18_BIT:
  //  Serial.println("18 bits");
    break;
  }

  // Set the mode of the ADC
  mcp.setMode(MODE_CONTINUOUS); // Options: MODE_CONTINUOUS, MODE_ONE_SHOT
  //Serial.print("Mode set to: ");
  switch (mcp.getMode())
  {
  case MODE_CONTINUOUS:
   // Serial.println("Continuous");
    break;
  case MODE_ONE_SHOT:
    //Serial.println("One-shot");
    break;
  }



  Serial.println("******************************************************");
}

// Function to perform dendrometer measurement
// void Dendro_Measure()
// {
//   // Set up Dendrometer
  
//  // Serial.println("Setting up I2C: ");
//   digitalWrite(DENDROMETER_ENABLE_PIN, HIGH); // turn the DENDRO on (HIGH is the voltage level)
//  Serial.println("Dendro En Done");

//   delay(2333);

//   // Initialize I2C communication
//   Wire.begin(I2C_SDA, I2C_SCL, 400000);

//   // Check if MCP3421 chip is found
//   if (!mcp.begin(0x68, &Wire))
//   {
//     Serial.println("Failed to find MCP3421 chip");
//     while (1)
//     {
//       delay(10); // Avoid a busy-wait loop
//     }
//   }
//  // Serial.println("MCP3421 Found!");

//   // Set the gain of the ADC
//   mcp.setGain(GAIN_1X);
//   //Serial.print("Gain set to: ");
//   switch (mcp.getGain())
//   {
//   case GAIN_1X:
//    // Serial.println("1X");
//     break;
//   case GAIN_2X:
//    // Serial.println("2X");
//     break;
//   case GAIN_4X:
//    // Serial.println("4X");
//     break;
//   case GAIN_8X:
//    // Serial.println("8X");
//     break;
//   }

//   // Set the resolution of the ADC
//   mcp.setResolution(RESOLUTION_18_BIT); // 240 SPS (12-bit)
//   //Serial.print("Resolution set to: ");
//   switch (mcp.getResolution())
//   {
//   case RESOLUTION_12_BIT:
//    // Serial.println("12 bits");
//     break;
//   case RESOLUTION_14_BIT:
//    // Serial.println("14 bits");
//     break;
//   case RESOLUTION_16_BIT:
//    // Serial.println("16 bits");
//     break;
//   case RESOLUTION_18_BIT:
//   //  Serial.println("18 bits");
//     break;
//   }

//   // Set the mode of the ADC
//   mcp.setMode(MODE_CONTINUOUS); // Options: MODE_CONTINUOUS, MODE_ONE_SHOT
//   //Serial.print("Mode set to: ");
//   switch (mcp.getMode())
//   {
//   case MODE_CONTINUOUS:
//    // Serial.println("Continuous");
//     break;
//   case MODE_ONE_SHOT:
//     //Serial.println("One-shot");
//     break;
//   }


// }


void Dendro_Measure() {

  long currentMillis = millis();

  if (mcp.isReady())
      {
        int32_t adcValue = mcp.readADC(); // Read ADC value
       //Serial.println(adcValue);
        arrDendroRaw[sampleCounterDendro] = adcValue;
        sampleCounterDendro++; // Increment the sample count 
      }
else {
return;
}
if (sampleCounterDendro==DENDRO_SAMPLE_SIZE)
{
//digitalWrite(DENDROMETER_ENABLE_PIN, LOW); // turn the DENDRO off (LOW is the voltage level);

  // Sort the ADC values in ascending order
  bubbleSort(arrDendroRaw, DENDRO_SAMPLE_SIZE);

  // Calculate the trimmed mean of the ADC values
    DendroRawTrimmed = trimmedMean(arrDendroRaw, DENDRO_SAMPLE_SIZE, DENDRO_TRIM_SIZE);

    //Serial.println("Raw, trimmed mean of ADC values: ");
   //Serial.println(DendroRawTrimmed);

 microns = 10000.0 - float(DendroRawTrimmed) * 10000.0 / 131071.0;

if (microns==10000.0)
{
  microns = 9999.99;
}


Serial.println("Dendrometer Done:");
Serial.print("Dendrometer Measurement: ");
Serial.println(microns);
Serial.println("******************************************************");
DENDRO_DONE = 1;
 
  return;
}
}
