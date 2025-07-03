#include "SapFlow_Driver.h"


HDC2080 sensor(ADDR);
HDC2080 sensor2(ADDR2);


float temp1;
float temp2;


uint8_t HEATER_STATE = BEFORE_HEAT;

std::vector<float> arrSapflowT1;
std::vector<float> arrSapflowT2;
int size = 10;
uint8_t sampleCounter = 0;

uint8_t startHP, endHP;

unsigned long tempSensorTimer = 0;

//Using millis instead of delay
unsigned long currentMillis;
unsigned long previousMillis = 0;

//Counter for millis since heat pulse was fired
unsigned long millisSinceHeatPulse = 0;

//Internal counter for starting/stopping heat pulse
unsigned long millisStartHeatPulse = 0;

//Millis value for last time heater was turned on
unsigned long previousHeaterOnTime = 0;

//Counter for millis since we started tracking reference temperatures
unsigned long millisStartReferenceTemp = 0;

unsigned long millisSinceReferenceTemp = 0;

boolean referenceTempRecorded = false;




void SFSetup(){
pinMode(HEAT_PIN_SWITCH, OUTPUT);
digitalWrite(HEAT_PIN_SWITCH, LOW);


//pinMode(SF_DENDRO_EN_PIN,OUTPUT);
digitalWrite(SF_DENDRO_EN_PIN,HIGH);

//delay(5000);

//digitalWrite(SF_DENDRO_EN_PIN,LOW);

//while(1);
  // Initialize I2C communication
  Wire.begin(I2C_SDA, I2C_SCL, 400000);

  if (wilo.i2cDeviceType==I2C_MUX){
  for (int port = 0; port < 8; port++)
  {
    if (i2cDevice[port].sensorType == SAPFLOW_SENSOR)
    {
     // Serial.print("Sapflow sensor exists on port: ");
     // Serial.println(port);
      I2C_Mux_SelectPort(port);
      delay(100);
      sensor.begin();
      sensor2.begin();

  // Begin with a device reset
      sensor.reset();
      sensor2.reset();

  // Configure Measurements
  sensor.setMeasurementMode(TEMP_AND_HUMID); // Set measurements to temperature and humidity
  sensor2.setMeasurementMode(TEMP_AND_HUMID);
  sensor.setRate(ONE_HZ); // Set measurement frequency to 1 Hz
  sensor2.setRate(ONE_HZ);
  sensor.setTempRes(FOURTEEN_BIT);
  sensor2.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);
  sensor2.setHumidRes(FOURTEEN_BIT);

  //begin measuring
  sensor.triggerMeasurement();
  sensor2.triggerMeasurement();

//Serial.println("Sapflow setup done on port: " + String(port));
//Serial.println("******************************************************");
    }
  }
  }else if(wilo.sfconnected){
        Serial.print("Sapflow sensor exists on default ");
      delay(100);
      sensor.begin();
      sensor2.begin();

  // Begin with a device reset
      sensor.reset();
      sensor2.reset();

  // Configure Measurements
  sensor.setMeasurementMode(TEMP_AND_HUMID); // Set measurements to temperature and humidity
  sensor2.setMeasurementMode(TEMP_AND_HUMID);
  sensor.setRate(ONE_HZ); // Set measurement frequency to 1 Hz
  sensor2.setRate(ONE_HZ);
  sensor.setTempRes(FOURTEEN_BIT);
  sensor2.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);
  sensor2.setHumidRes(FOURTEEN_BIT);

  //begin measuring
  sensor.triggerMeasurement();
  sensor2.triggerMeasurement();

Serial.println("Sapflow setup done on default port");
Serial.println("******************************************************");
  } else {
    Serial.println("No sapflow sensor connected, setting SF_DONE");
    SF_DONE = 1;
  }

}

void SF_Measure(){
 currentMillis = millis();

  //start reading temperature for baseline reference check
  if (HEATER_STATE==0 && millisStartReferenceTemp == 0)
  {
    Serial.println(F("Starting to read reference sapflow temperatures..."));
    millisStartReferenceTemp = currentMillis;
  }

  if (HEATER_STATE==0 && currentMillis - millisStartReferenceTemp >= SAMPLE_TIME_BEFORE_HP && previousHeaterOnTime == 0)
  {
    previousMillis = currentMillis;
    millisStartHeatPulse = currentMillis;
    Serial.println(F("Done reading reference sapflow temperatures. Turning heating element on."));
    // SDI12_Setup();
    // SDI12_CONNECTED = SDI12_Check();
    digitalWrite(SDI12_EN_PIN, HIGH);
    HEATER_STATE = DURING_HEAT;
    Serial.println(F("Heater and SDI-12 ON"));
     Serial.println("******************************************************");
    
    startHP = sampleCounter;
    previousHeaterOnTime = currentMillis;
  }
  // //turn on heating element every 30 minutes
  // else if (digitalRead(HEAT_PIN_SWITCH) == LOW && currentMillis - millisStartReferenceTemp >= 1800000)
  // {
  //   previousMillis = 0;
  //   previousHeaterOnTime = 0;
  //   millisStartReferenceTemp = 0;
  // }

  //turn off heating element after it has been on for >= 15 seconds
  if (digitalRead(SDI12_EN_PIN) == HIGH && currentMillis - millisStartHeatPulse >= SAMPLE_TIME_DURING_HP)
  {
    //digitalWrite(HEAT_PIN_SWITCH, LOW);
    SDI12_Shutdown();
    HEATER_STATE = AFTER_HEAT;
    Serial.println(F("Heater OFF"));
     Serial.println("******************************************************");
    
  }

  millisSinceHeatPulse = currentMillis - previousHeaterOnTime;
  millisSinceReferenceTemp = currentMillis - millisStartReferenceTemp;

  
   if (millisSinceHeatPulse > (SAMPLE_TIME_DURING_HP+SAMPLE_TIME_AFTER_HP)) {
    Serial.println(" All sapflow measurements done done!");


    if(wilo.i2cDeviceType==I2C_MUX){
    for(int port = 0; port < 8; port++)
    {
      if (i2cDevice[port].sensorType == SAPFLOW_SENSOR)
      {
        calculateAverages(port);
      }
    }

   }else{
    calculateAveragesSFWiloDefault();
   }


   SF_DONE = 1;
     return;}

  if (millis() - tempSensorTimer >= 1000)
  {

    if(wilo.i2cDeviceType==I2C_MUX){
for (int port = 0; port < 8; port++)
      {
      
        if (i2cDevice[port].sensorType == SAPFLOW_SENSOR)
        {
          I2C_Mux_SelectPort(port);
          //delay(50);
          
          temp1 = sensor.readTemp();
          temp2 = sensor2.readTemp();

          Serial.print("Measurement on port: ");
          Serial.println(port);
          Serial.print("Sensor 1 Temperature (C): ");
          Serial.println(temp1);
          Serial.print(" Sensor 2 Temperature (C): ");
          Serial.println(temp2);
          Serial.print(" Heater State: ");
          Serial.println(HEATER_STATE);

        passValuesToStruct(port, temp1, temp2, HEATER_STATE);
        }
        else
        {
          Serial.println("No saplfow sensor connected");
         
        }
      }
    }else {
        
          temp1 = sensor.readTemp();
          temp2 = sensor2.readTemp();

          Serial.print("Measurement on default port: ");
          Serial.print("Sensor 1 Temperature (C): ");
          Serial.println(temp1);
          Serial.print(" Sensor 2 Temperature (C): ");
          Serial.println(temp2);
          Serial.print(" Heater State: ");
          Serial.println(HEATER_STATE);

          passValuesToStructDefaultWilo(temp1, temp2, HEATER_STATE);
          
    }

    tempSensorTimer = millis();
    Serial.print(F(" millisSinceHeatPulse: "));
    Serial.println(millisSinceHeatPulse);

    
      Serial.print("sampleCounter Current Size:");
    Serial.println(sampleCounter);

     Serial.println("******************************************************");
    sampleCounter++;
   
  } 
    
   }
  

// void testRead(){
//    temp1 = sensor.readTemp();
//     Serial.print("Sensor 1 Temperature (C): ");
//     Serial.print(temp1);
//     temp2 = sensor2.readTemp();
//     Serial.print(" Sensor 2 Temperature (C): ");
// }





// float calculateHPV(const std::vector<float>& arrSapflowT1, const std::vector<float>& arrSapflowT2, uint8_t StartHP, uint8_t endHP) {
//     // Check if the Start and End indices are within the bounds of the vectors
//     if (StartHP >= arrSapflowT1.size() || endHP >= arrSapflowT1.size() || StartHP > endHP) {
//         return -1.0f;  // Return an error value if indices are out of bounds or invalid
//     }

//     size_t dataSize = arrSapflowT1.size();

  
//     for (size_t i = 0; i < StartHP; ++i) {
//         avgT1Before += arrSapflowT1[i];
//         avgT2Before += arrSapflowT2[i];
//     }
//     avgT1Before /= StartHP;
//     avgT2Before /= StartHP;


//     Serial.println("Avg T1 Before:");
//     Serial.println(avgT1Before);
//     Serial.println("Avg T2 Before:");
//     Serial.println(avgT2Before);

//     // Calculate average temperature for the "during" heat pulse period

    
//     for (size_t i = StartHP; i <= endHP; ++i) {
//         avgT1During += arrSapflowT1[i];
//         avgT2During += arrSapflowT2[i];
//     }
//     avgT1During /= (endHP - StartHP + 1);
//     avgT2During /= (endHP - StartHP + 1);

//     Serial.println("Avg T1 During:");
//     Serial.println(avgT1During);
//     Serial.println("Avg T2 During:");
//     Serial.println(avgT2During);

//     // Calculate average temperature for the "after" heat pulse period
//       // Calculate average temperature for the "before" heat pulse period (baseline)
  
//     for (size_t i = endHP + 1; i < dataSize; ++i) {
//         avgT1After += arrSapflowT1[i];
//         avgT2After += arrSapflowT2[i];
//     }
//     avgT1After /= (dataSize - endHP - 1);
//     avgT2After /= (dataSize - endHP - 1);

// Serial.println("Avg T1 After:");
//     Serial.println(avgT1After);
//     Serial.println("Avg T2 After:");
//     Serial.println(avgT2After);

//     // // Calculate temperature differences for each period
//     // float deltaT1Before = avgT1During - avgT1Before;
//     // float deltaT2Before = avgT2During - avgT2Before;

//     // float deltaT1During = avgT1During - avgT1Before;  // Calculate difference for "during" period
//     // float deltaT2During = avgT2During - avgT2Before;  // Calculate difference for "during" period

//     // float deltaT1After = avgT1After - avgT1During;
//     // float deltaT2After = avgT2After - avgT2During;


//     float deltaT1AfterBefore = avgT1After - avgT1Before;
//     float deltaT2AfterBefore = avgT2After - avgT2Before;

//     Serial.println("Delta T1 After minus Before:");
//     Serial.println(deltaT1AfterBefore);
//     Serial.println("Delta T2 After minus Before:");
//     Serial.println(deltaT2AfterBefore);


//     //Marshall formula values
// float k = 0.25; //mm^2
// float x  = 6.0; // 6 mm 

//     float k_over_x = k / x;

//     Serial.println("k/x:");
//     Serial.println(k_over_x);

//     float Calc_HPV = 0.0f;


// if (avgT1Before == -40.0f || avgT2Before == -40.0f || avgT1During == -40.0f || avgT2During == -40.0f || avgT1After == -40.0f || avgT2After == -40.0f){
//    Serial.println("Sapflow sensor not connected or faulty");
//     Calc_HPV = 1111.11f;
//     avgT1Before = 1111.11f;
//     avgT2Before = 1111.11f;
//     avgT1During = 1111.11f;
//     avgT2During = 1111.11f;
//     avgT1After = 1111.11f;
//     avgT2After = 1111.11f;
//     deltaT1AfterBefore = 1111.11f;
//     deltaT2AfterBefore = 1111.11f;

//    } else{
//     Calc_HPV = k_over_x * (log( deltaT2AfterBefore/deltaT1AfterBefore)) * 3600.0; //mm/h
//    }
 
//     return Calc_HPV;
// }
