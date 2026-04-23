#include "SapFlow_Driver.h"


HDC2080 sensor(ADDR);
HDC2080 sensor2(ADDR2);


float temp1;
float temp2;

float arrTemp1Median[SAPFLOW_SAMPLE_SIZE];
float arrTemp2Median[SAPFLOW_SAMPLE_SIZE];


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

//Timestamp for when the post-heat delay started
unsigned long millisStartWaitAfterHeat = 0;

//Counter for millis since we started tracking reference temperatures
unsigned long millisStartReferenceTemp = 0;

unsigned long millisSinceReferenceTemp = 0;

boolean referenceTempRecorded = false;




void SFSetup(){
// pinMode(HEAT_PIN_SWITCH, OUTPUT);
// digitalWrite(HEAT_PIN_SWITCH, LOW);

//pinMode(SF_DENDRO_EN_PIN,OUTPUT);
if(!digitalRead(SF_DENDRO_EN_PIN)){
digitalWrite(SF_DENDRO_EN_PIN,HIGH);
}

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
      delay(50);
      sensor.begin();
      sensor2.begin();

  // Begin with a device reset
      sensor.reset();
      sensor2.reset();

  // Configure Measurements
  sensor.setMeasurementMode(TEMP_ONLY); // Set measurements to temperature and humidity
  sensor2.setMeasurementMode(TEMP_ONLY);
  sensor.setRate(TWO_HZ); // Set measurement frequency to 2 Hz
  sensor2.setRate(TWO_HZ);
  sensor.setTempRes(FOURTEEN_BIT);
  sensor2.setTempRes(FOURTEEN_BIT);
  // sensor.setHumidRes(FOURTEEN_BIT);
  // sensor2.setHumidRes(FOURTEEN_BIT);

  //begin measuring
  sensor.triggerMeasurement();
  sensor2.triggerMeasurement();

//Serial.println("Sapflow setup done on port: " + String(port));
//Serial.println("******************************************************");
    }
  }
  }else if(wilo.sfconnected){
        Serial.print("Sapflow sensor exists on default ");
      //delay(100);
      sensor.begin();
      sensor2.begin();

  // Begin with a device reset
      sensor.reset();
      sensor2.reset();

  // Configure Measurements
  sensor.setMeasurementMode(TEMP_ONLY); // Set measurements to temperature and humidity
  sensor2.setMeasurementMode(TEMP_ONLY);
  sensor.setRate(TWO_HZ); // Set measurement frequency to 1 Hz
  sensor2.setRate(TWO_HZ);
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
  if (HEATER_STATE==BEFORE_HEAT && millisStartReferenceTemp == 0)
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
 
  //turn off heating element after it has been on
  if (digitalRead(SDI12_EN_PIN) == HIGH && currentMillis - millisStartHeatPulse >= SAMPLE_TIME_DURING_HP)
  {
    SDI12_Shutdown();
    HEATER_STATE = WAIT_AFTER_HEAT;
    millisStartWaitAfterHeat = currentMillis;
    Serial.println(F("Heater OFF. Waiting 25s before after-heat sampling."));
    Serial.println("******************************************************");
  }

  // 25-second delay before after-heat sampling begins
  if (HEATER_STATE == WAIT_AFTER_HEAT && currentMillis - millisStartWaitAfterHeat >= SF_DELAY)
  {
    HEATER_STATE = AFTER_HEAT;
    Serial.println(F("Wait period over. Starting after-heat measurements."));
    Serial.println("******************************************************");
  }

  millisSinceHeatPulse = currentMillis - previousHeaterOnTime;
  millisSinceReferenceTemp = currentMillis - millisStartReferenceTemp;

  
   if (previousHeaterOnTime != 0 && millisSinceHeatPulse > (SAMPLE_TIME_DURING_HP+SF_DELAY+SAMPLE_TIME_AFTER_HP)) {
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

  if (millis() - tempSensorTimer >= 550 && HEATER_STATE != WAIT_AFTER_HEAT)
  {

    if(wilo.i2cDeviceType==I2C_MUX){

  Wire.beginTransmission(I2C_MUX_ADDR);
  if (Wire.endTransmission() == 0) {
    // Serial.println("I2C MUX found at address: " + String(I2C_MUX_ADDR, HEX));
    
  }
      
    
for (int port = 0; port < 8; port++)
      {
      
        if (i2cDevice[port].sensorType == SAPFLOW_SENSOR)
        {
         
          I2C_Mux_SelectPort(port);
          delay(50);
          
          for(uint8_t sample =0; sample < SAPFLOW_SAMPLE_SIZE; sample++){
            arrTemp1Median[sample] = sensor.readTemp();
            arrTemp2Median[sample] = sensor2.readTemp();
            //delay(20);
          }

          temp1 = trimmedMean(arrTemp1Median, SAPFLOW_SAMPLE_SIZE,5);
          temp2 = trimmedMean(arrTemp2Median, SAPFLOW_SAMPLE_SIZE,5);

         // Serial.println("Getting DateTime from RTC...");
          String SFDate  = getDateTimeRTC(GET_DATE); // Get DATE
          String SFTime = getDateTimeRTC(GET_TIME); // Get TIME

          SFSetup();

          std::string temp1Str = std::to_string(temp1);
          std::string temp2Str = std::to_string(temp2);

          std::string BootCountStr = std::to_string(bootCount);

        

          Serial.print("Measurement on port: ");
          Serial.println(port);
          Serial.print("Sensor 1 Temperature (C): ");
          Serial.println(temp1);
          Serial.print(" Sensor 2 Temperature (C): ");
          Serial.println(temp2);
          Serial.print(" Heater State: ");
          Serial.println(HEATER_STATE);
          Serial.println("Timestamp: " + SFDate + " " + SFTime);


          String SDLine = SFDate + "," + SFTime + "," + String(temp1Str.c_str()) + "," + String(temp2Str.c_str()) + "," + String(HEATER_STATE).c_str()+ "," + BootCountStr.c_str();
          writeToMuxFile(port, SAPFLOW_SENSOR, SDLine.c_str());

  
        passValuesToStruct(port, temp1, temp2, HEATER_STATE);
        }
        else
        {
          Serial.println("No saplfow sensor connected");
         
        }
      }
    }else {

    
        
           for(uint8_t sample =0; sample < SAPFLOW_SAMPLE_SIZE; sample++){
            arrTemp1Median[sample] = sensor.readTemp();
            arrTemp2Median[sample] = sensor2.readTemp();
          }

          temp1 = trimmedMean(arrTemp1Median, SAPFLOW_SAMPLE_SIZE,5);
          temp2 = trimmedMean(arrTemp2Median, SAPFLOW_SAMPLE_SIZE,5);

       
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
  

void SFtestRead(){

  if (!digitalRead(SF_DENDRO_EN_PIN))
  {
   digitalWrite(SF_DENDRO_EN_PIN,HIGH);
   delay(100);
  }
  




    for (int port = 0; port < 8; port++)
      {
      
        if (i2cDevice[port].sensorType == SAPFLOW_SENSOR)
        {
          I2C_Mux_SelectPort(port);
          //delay(50);
        

          for(uint8_t sample =0; sample < SAPFLOW_SAMPLE_SIZE; sample++){
            arrTemp1Median[sample] = sensor.readTemp();
            arrTemp2Median[sample] = sensor2.readTemp();
          }



          temp1 = trimmedMean(arrTemp1Median, SAPFLOW_SAMPLE_SIZE,5);
          temp2 = trimmedMean(arrTemp2Median, SAPFLOW_SAMPLE_SIZE,5);

  
          Serial.print("Measurement on port: ");
          Serial.println(port);
          Serial.print("Sensor 1 Temperature (C): ");
          Serial.println(temp1);
          Serial.print(" Sensor 2 Temperature (C): ");
          Serial.println(temp2);
        }
        else
        {
          Serial.println("No saplfow sensor connected");
         
        }
}
}


