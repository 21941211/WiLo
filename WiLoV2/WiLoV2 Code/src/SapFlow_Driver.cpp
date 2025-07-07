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


digitalWrite(HEAT_PULSE_EN_PIN, LOW);

pinMode(I2C_PORT_EN_PIN,OUTPUT);
digitalWrite(I2C_PORT_EN_PIN,HIGH);

delay(200);

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
        Serial.print("Sapflow sensor exists on default port");
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
    digitalWrite(HEAT_PULSE_EN_PIN, HIGH);
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
  if (digitalRead(HEAT_PULSE_EN_PIN) == HIGH && currentMillis - millisStartHeatPulse >= SAMPLE_TIME_DURING_HP)
  {
    digitalWrite(HEAT_PULSE_EN_PIN, LOW);
   // SDI12_Shutdown();
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
  
