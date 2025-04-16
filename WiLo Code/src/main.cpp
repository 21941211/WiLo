#include <Arduino.h>
#include "LoRa_Driver.h"
#include "DHT22_Driver.h"
#include "SD_Driver.h"
#include "DeepSleep_Driver.h"
#include "DataProcessing_Driver.h"
#include "SoilMoisture_Driver.h"
#include "LinPotDendro_Driver.h"
#include "BatteryLevel_Driver.h"
#include <Wire.h>
#include "DS18B20_Driver.h"
#include "SDI-12_Driver.h"
#include "MeasureFlags.h"
#include "SapFlow_Driver.h"
#include "driver/rtc_io.h"
#include "debug.h"

#define PULLDOWN_GPIO GPIO_NUM_5 // Only RTC IO are allowed

uint8_t SDWRITE_DONE = 0;

uint8_t SDI12_SETUP_COMPLETE = 0;

void setup()
{

  pinMode(GPIO_NUM_5, OUTPUT);
  digitalWrite(GPIO_NUM_5, LOW);

  // Slow down CPU for lower power usage
  setCpuFrequencyMhz(40);
  Serial.begin(115200);
  delay(500);



  Serial.println("Starting");
    Serial.println("Checking flags and boot count: ");
  if (bootCount > 999999)
  {
    bootCount = 0;
  }
  else if(!MEASURE_COMPLETE){
    bootCount++;
  }

    Serial.println("Boot number: " + String(bootCount));
    Serial.println("SDI12-Connection Status (0 = NC, 1 = C): " +String(SDI12_CONNECTED));
    Serial.println("SDI12-Connection Type (0 = 60cm, 1 = 90cm, 2 = CS655, 3 = none): "+ String(SDI12_TYPE));
    Serial.println("Measurement transmitted in previous cycle (0 = yes, 1 = no): " + String(MEASURE_COMPLETE));
  Serial.println("******************************************************");


  

  pinMode(SD_ENABLE_PIN, OUTPUT);
  pinMode(LORA_CS_PIN, OUTPUT);
  pinMode(DEBUG_LED_PIN, OUTPUT);
  pinMode(DHT22_SM_ENABLE_PIN, OUTPUT);
  

#ifdef ENABLE_SD
  SDSetup();

#else
  Serial.println("SD Card Disabled, using default LoRa parameters");

#endif

  if (BATTERY_LOW == 1)
  {
    measBat();
    // this will never run if the battery is low
    BATTERY_LOW = 0;
  }

  if (MEASURE_COMPLETE)
  {
    LoRaSetup();
    decodePayload();
    os_runloop();
  }

#ifdef ENABLE_MEASURE
  pinMode(DHT22_SM_ENABLE_PIN, OUTPUT);
  pinMode(SF_DENDRO_EN_PIN, OUTPUT);
#endif

#ifdef ENABLE_SDI12_TESTING
  while (!SDI12_DONE)
  {
    if (!SDI12_CONNECTED)
    {
      SDI12_Setup();
      //while(1){
      SDI12_CONNECTED = SDI12_Check();

        #ifdef ENABLE_CS655_TESTING
         while(!testCS655());     

        #endif

       InfiniteStop();

      delay(1000);
      }

    // }

    if (!SDI12_CONNECTED)
    {
      SDI12_DONE = 1;
    }
    else
    {
      while (!SDI12_Measure(SDI12_SOIL_MOISTURE));
      while (!SDI12_Measure(SDI12_TEMPERATURE));
      SDSetup_SDI12();
     
    }
  }
  InfiniteStop();
#endif

#if defined(ENABLE_SD) && defined(ENABLE_SDI12)
  if (SDI12_CONNECTED)
  {
   SDSetup_SDI12();
  }
#endif

#ifdef ENABLE_LORA
  if (MEASURE_COMPLETE)
  {
    LoRaSetup();
  }
#endif

#ifdef ENABLE_LORA_TEST
  {
    LoRaSetup();
  }
#endif

  digitalWrite(DEBUG_LED_PIN, HIGH);

  //digitalWrite(GPIO_NUM_2, HIGH); // turn the DENDRO on (HIGH is the voltage level)
  //delay(3000);
  dendroSetup();

  // pinMode(I2C_SCL,INPUT);
  //   pinMode(I2C_SDA,INPUT);



  //gpio_reset_pin(GPIO_NUM_2);
  //pinMode(GPIO_NUM_2,OUTPUT);
  //  digitalWrite(GPIO_NUM_2,LOW);

  // delay(3000);
  //   digitalWrite(GPIO_NUM_2,LOW);

  // delay(3000);
  //   digitalWrite(GPIO_NUM_2,LOW);

  // delay(3000);
  // while (1);
  SFSetup();

  digitalWrite(DEBUG_LED_PIN, LOW); // turn the LED off (HIGH is the voltage level)
}

void loop()
{

#ifdef ENABLE_CS655_TESTING
testCS655();
#endif


#ifndef ENABLE_CS655_TESTING
#ifndef ENABLE_SD
  MEASURE_COMPLETE = 1;
#endif

  if (DHT22_DONE && SM_DONE)
  {
    digitalWrite(DHT22_SM_ENABLE_PIN, LOW); // turn the LED on (HIGH is the voltage level)
  }
  else if (digitalRead(DHT22_SM_ENABLE_PIN) == LOW)
  {
    digitalWrite(DHT22_SM_ENABLE_PIN, HIGH); // turn the LED on (HIGH is the voltage level))
    DHTSetup();
    Serial.println("DHT22, SM and SF now ON");
    Serial.println("******************************************************");
  }

  if (!MEASURE_COMPLETE)
  {

    if (!BATT_DONE)
      measBat();
    if (!DHT22_DONE)
      DHT_Measure();
    if (!SM_DONE)
      SM_Measure();
    if (!ST_DONE)
      readDS18B20();
    if (!DENDRO_DONE)
      Dendro_Measure();
    if (!SF_DONE)
      SF_Measure();

    if (DENDRO_DONE && SF_DONE)
    {
      
   
     // gpio_reset_pin(GPIO_NUM_2);
         //pinMode(GPIO_NUM_2,OUTPUT);
    //  digitalWrite(GPIO_NUM_2,LOW);
    
     digitalWrite(SF_DENDRO_EN_PIN, LOW);   
    pinMode(I2C_SCL,INPUT);
    pinMode(I2C_SDA,INPUT);
      Serial.println("Turning off Dendro and SF: ");
    
      if(!SDI12_CONNECTED){
 rtc_gpio_hold_dis(GPIO_NUM_5);
        gpio_reset_pin(GPIO_NUM_5);
      }
     
      Serial.println("This is test code");
    }

    if (HEATER_STATE && !SDI12_DONE)
    {
      if (!SDI12_SETUP_COMPLETE)
      {
        SDI12_Setup();
        SDI12_CONNECTED = SDI12_Check();
        SDI12_SETUP_COMPLETE = 1;
      }

      if (!SDI12_CONNECTED)
      {
        SDI12_DONE = 1;
      }
      else
      {
        switch (SDI12_MEASURE_STATE)
        {
        case 0:
          if (SDI12_Measure(SDI12_SOIL_MOISTURE))
            SDI12_MEASURE_STATE = 1;
          break;
        case 1:
          if (SDI12_Measure(SDI12_TEMPERATURE))
          {
            SDI12_MEASURE_STATE = 3;
            SDI12_DONE = 1;
            Serial.println("SDI12 Measurements Done");
          }
          case 2:
          if(testCS655()) {
             SDI12_DONE = 1;
            Serial.println("CS655 Measurements Done");
          }
          break;
        }
      }
    }

    if (BATT_DONE && SM_DONE && ST_DONE && DENDRO_DONE && SF_DONE && SDI12_DONE)
    {
      Serial.println("All Measurements Done");
      Serial.println("******************************************************");

      MEASURE_COMPLETE = 1;
    }
  }
  else
  {

    if (!SDWRITE_DONE)
    {
#ifdef ENABLE_SD

      writeToSD();

      if (SDI12_CONNECTED)
      {
        writeToSD_SDI12();
      }

#endif

      LoRaSetup();

#ifdef ENABLE_SD
      decodePayload();
      
#endif

      SDWRITE_DONE = 1;
    }

#ifdef ENABLE_LORA

    os_runloop_once();
   // delay(100);
#endif
  }
  #endif
}
