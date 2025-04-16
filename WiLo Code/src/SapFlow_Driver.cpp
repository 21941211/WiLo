#include "SapFlow_Driver.h"



HDC2080 sensor(ADDR);
HDC2080 sensor2(ADDR2);

  float avgT1Before = 0.0f;
  float avgT2Before = 0.0f;
  float avgT1During = 0.0f;
  float avgT2During = 0.0f;
  float avgT1After = 0.0f;
  float avgT2After = 0.0f;
  float HPV = 0.0f;

uint8_t HEATER_STATE = 0;

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




float temp1;
float temp2;

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

Serial.println("Sapflow setup done");

Serial.println("******************************************************");
}

void SF_Measure(){
 currentMillis = millis();

  //start reading temperature for baseline reference check
  if (!HEATER_STATE && millisStartReferenceTemp == 0)
  {
    Serial.println(F("Starting to read reference sapflow temperatures..."));
    millisStartReferenceTemp = currentMillis;
  }

  if (!HEATER_STATE && currentMillis - millisStartReferenceTemp >= 10000 && previousHeaterOnTime == 0)
  {
    previousMillis = currentMillis;
    millisStartHeatPulse = currentMillis;
    Serial.println(F("Done reading reference sapflow temperatures. Turning heating element on."));
    // SDI12_Setup();
    // SDI12_CONNECTED = SDI12_Check();
    //digitalWrite(HEAT_PIN_SWITCH, HIGH);
    HEATER_STATE = 1;
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

  //turn off heating element after it has been on for >= 20 seconds
  if (digitalRead(SDI12_EN_PIN) == HIGH && currentMillis - millisStartHeatPulse >= 20000)
  {
    //digitalWrite(HEAT_PIN_SWITCH, LOW);
    SDI12_Shutdown();
    HEATER_STATE = 0;
    Serial.println(F("Heater OFF"));
     Serial.println("******************************************************");
    endHP = sampleCounter;
  }

  millisSinceHeatPulse = currentMillis - previousHeaterOnTime;
  millisSinceReferenceTemp = currentMillis - millisStartReferenceTemp;

  
   if (millisSinceHeatPulse > 30000) {

Serial.print("ArrSapflowT1 Size:");

int size = arrSapflowT1.size();

Serial.println(size);

    for (int i = 0; i < size; i++)
    {
      Serial.print("T1[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(arrSapflowT1[i]);
      Serial.print(" T2[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(arrSapflowT2[i]);
      
    }
    Serial.print("Heat pulse start index: ");
    Serial.println(startHP);

    Serial.print("Heat pulse end index: ");
    Serial.println(endHP);

    Serial.println("Sapflow done:");
   // digitalWrite(SF_DENDRO_EN_PIN,LOW);
   // gpio_reset_pin(SF_DENDRO_EN_PIN);
    //while(1);
    Serial.println("Estimated HPV using Marshall formulab (scaled by 100): ");
    HPV = calculateHPV(arrSapflowT1, arrSapflowT2, startHP, endHP)*100.0;
    if (isnan(HPV))
    {
      HPV=9999.99;
    }else if(HPV < 0){
      HPV = 0.0;
    }
    
    Serial.println(HPV);
     SF_DONE = 1;
     return;
   }

  if (millis() - tempSensorTimer >= 1000)
  {
    arrSapflowT1.resize(sampleCounter+1);
    arrSapflowT2.resize(sampleCounter+1);



int size = arrSapflowT1.size();
Serial.print("New arr size: ");
Serial.println(size);


    tempSensorTimer = millis();
    temp1 = sensor.readTemp();
    Serial.print("Sensor 1 Temperature (C): ");
    Serial.print(temp1);
    temp2 = sensor2.readTemp();
    Serial.print(" Sensor 2 Temperature (C): ");
    Serial.print(temp2);
    arrSapflowT1[sampleCounter] = temp1;
    arrSapflowT2[sampleCounter] = temp2;
    Serial.print(F(" millisSinceHeatPulse: "));
    Serial.println(millisSinceHeatPulse);

    
      Serial.print("sampleCounter Current Size:");
    Serial.println(sampleCounter);

     Serial.println("******************************************************");
    sampleCounter++;
   
  } 
    
}

void testRead(){
   temp1 = sensor.readTemp();
    Serial.print("Sensor 1 Temperature (C): ");
    Serial.print(temp1);
    temp2 = sensor2.readTemp();
    Serial.print(" Sensor 2 Temperature (C): ");
}



float calculateHPV(const std::vector<float>& arrSapflowT1, const std::vector<float>& arrSapflowT2, uint8_t StartHP, uint8_t endHP) {
    // Check if the Start and End indices are within the bounds of the vectors
    if (StartHP >= arrSapflowT1.size() || endHP >= arrSapflowT1.size() || StartHP > endHP) {
        return -1.0f;  // Return an error value if indices are out of bounds or invalid
    }

    size_t dataSize = arrSapflowT1.size();

  
    for (size_t i = 0; i < StartHP; ++i) {
        avgT1Before += arrSapflowT1[i];
        avgT2Before += arrSapflowT2[i];
    }
    avgT1Before /= StartHP;
    avgT2Before /= StartHP;


    Serial.println("Avg T1 Before:");
    Serial.println(avgT1Before);
    Serial.println("Avg T2 Before:");
    Serial.println(avgT2Before);

    // Calculate average temperature for the "during" heat pulse period

    
    for (size_t i = StartHP; i <= endHP; ++i) {
        avgT1During += arrSapflowT1[i];
        avgT2During += arrSapflowT2[i];
    }
    avgT1During /= (endHP - StartHP + 1);
    avgT2During /= (endHP - StartHP + 1);

    Serial.println("Avg T1 During:");
    Serial.println(avgT1During);
    Serial.println("Avg T2 During:");
    Serial.println(avgT2During);

    // Calculate average temperature for the "after" heat pulse period
      // Calculate average temperature for the "before" heat pulse period (baseline)
  
    for (size_t i = endHP + 1; i < dataSize; ++i) {
        avgT1After += arrSapflowT1[i];
        avgT2After += arrSapflowT2[i];
    }
    avgT1After /= (dataSize - endHP - 1);
    avgT2After /= (dataSize - endHP - 1);

Serial.println("Avg T1 After:");
    Serial.println(avgT1After);
    Serial.println("Avg T2 After:");
    Serial.println(avgT2After);

    // // Calculate temperature differences for each period
    // float deltaT1Before = avgT1During - avgT1Before;
    // float deltaT2Before = avgT2During - avgT2Before;

    // float deltaT1During = avgT1During - avgT1Before;  // Calculate difference for "during" period
    // float deltaT2During = avgT2During - avgT2Before;  // Calculate difference for "during" period

    // float deltaT1After = avgT1After - avgT1During;
    // float deltaT2After = avgT2After - avgT2During;


    float deltaT1AfterBefore = avgT1After - avgT1Before;
    float deltaT2AfterBefore = avgT2After - avgT2Before;

    Serial.println("Delta T1 After minus Before:");
    Serial.println(deltaT1AfterBefore);
    Serial.println("Delta T2 After minus Before:");
    Serial.println(deltaT2AfterBefore);


    //Marshall formula values
float k = 0.25/1000000.0; //m^2
float x  = 6.0/1000.0; // 6 mm to m

    float k_over_x = k / x;

    Serial.println("k/x:");
    Serial.println(k_over_x);

    float Calc_HPV = k_over_x * (log( deltaT2AfterBefore/deltaT1AfterBefore)) * 3600.0;

 
    return Calc_HPV;
}
