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
#include "OTA_Driver.h"
#include "esp_wifi.h"
#include "I2C_Driver.h"
#include "payload_Config.h"
#include "defaultWiLoData.h"
#include "RTC_Driver.h"

#define PULLDOWN_GPIO GPIO_NUM_5 // Only RTC IO are allowed

// uint8_t payloadConfigTest[3]= {0, 0, 0}; // Initialize payloadConfig to 0

uint8_t SDWRITE_DONE = 0;

uint8_t SDI12_SETUP_COMPLETE = 0;

void setup()
{

  pinMode(GPIO_NUM_5, OUTPUT);
  digitalWrite(GPIO_NUM_5, LOW);

  Serial.begin(115200);
  delay(2000);

#ifdef WIFI_ENABLED
  if (!OTA_Window_Missed)
  {
    if (setupOTA())
    {
      LoopOTA();
      OTA_Window_Missed = 1;
      esp_sleep_enable_timer_wakeup(2 * uS_TO_S_FACTOR);
      disableWiFi();
      esp_wifi_deinit(); // deeper cleanup, if needed
      Serial.println("OTA Window Missed, going to sleep for 2 seconds");
      delay(1000);
      esp_deep_sleep_start();
    }
    else
    {
      OTA_Window_Missed = 1;
      disableWiFi();
      esp_sleep_enable_timer_wakeup(2 * uS_TO_S_FACTOR);
      esp_wifi_deinit(); // deeper cleanup, if needed
      Serial.println("OTA Window Missed, going to sleep for 2 seconds");
      delay(1000);
      esp_deep_sleep_start();
    }
  }

#endif

  Serial.println("Starting");

  // setupRTC();

  setCpuFrequencyMhz(40);
  delay(500);

  Serial.println("Checking flags and boot count: ");
  if (bootCount > 999999)
  {
    bootCount = 0;
  }
  else if (!MEASURE_COMPLETE)
  {
    bootCount++;
  }

  Serial.println("Boot number: " + String(bootCount));
  Serial.println("SDI12-Connection Status (0 = NC, 1 = C): " + String(SDI12_CONNECTED));
  Serial.println("SDI12-Connection Type (0 = 60cm, 1 = 90cm, 2 = CS655, 3 = none): " + String(SDI12_TYPE));
  Serial.println("Measurement transmitted in previous cycle (0 = yes, 1 = no): " + String(MEASURE_COMPLETE));
  Serial.println("******************************************************");

  pinMode(SF_DENDRO_EN_PIN, OUTPUT);
  pinMode(SDI12_EN_PIN, OUTPUT);
  digitalWrite(SF_DENDRO_EN_PIN, HIGH);
  pinMode(SD_ENABLE_PIN, OUTPUT);
  pinMode(LORA_CS_PIN, OUTPUT);
  pinMode(DEBUG_LED_PIN, OUTPUT);
  pinMode(DHT22_SM_ENABLE_PIN, OUTPUT);

  //  SDI12_Setup();
  //   SDI12_CONNECTED = SDI12_Check();
  //   SDI12_SETUP_COMPLETE = 1;

  //   while(1);

#ifdef ENABLE_SD
  Serial.println("Initalising SD Card: ");
  InitialSDSetup();

#else
  Serial.println("SD Card Disabled, using default LoRa parameters");

#endif

  pinMode(SF_DENDRO_EN_PIN, OUTPUT);
  pinMode(SDI12_EN_PIN, OUTPUT);
  digitalWrite(SF_DENDRO_EN_PIN, HIGH);
  Serial.println("Turning on I2C: ");
  // Setup all I2C Devices
  initialiseI2CMuxStructs();
  I2Csetup();
  dendroSetup();

  // SFSetup();  MOVED DOWN
  // delay(1000);

  printPayloadConfig();

  if (BATTERY_LOW == 1)
  {
    measBat();
    // this will never run if the battery is low
    BATTERY_LOW = 0;
  }

  if (MEASURE_COMPLETE)
  {
    LoRaSetup();
    decodePayload(payload);
    decodePayload(payloadReduced);
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
      // while(1){
      SDI12_CONNECTED = SDI12_Check();

#ifdef ENABLE_CS655_TESTING
      while (!testCS655())
        ;

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
      while (!SDI12_Measure(SDI12_SOIL_MOISTURE))
        ;
      while (!SDI12_Measure(SDI12_TEMPERATURE))
        ;
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

  if (!SDI12_CONNECTED)
  {
    rtc_gpio_hold_dis(GPIO_NUM_5);
    gpio_reset_pin(GPIO_NUM_5);
  }

  // setSDI12DummyValues(SDI12_DD_90);

  SFSetup();
  delay(1000);
  // while(1){
  // SFtestRead();
  // delay(1000);
  // }
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
    digitalWrite(DHT22_SM_ENABLE_PIN, LOW);
  }
  else if (digitalRead(DHT22_SM_ENABLE_PIN) == LOW)
  {
    digitalWrite(DHT22_SM_ENABLE_PIN, HIGH);
    DHTSetup();
    Serial.println("DHT22, and SM now ON");
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

    if (!SF_DONE)
    {
      SF_Measure();
    }

    if (SF_DONE && !DENDRO_DONE)
    {
      Dendro_Measure();
    }
    if (DENDRO_DONE && SF_DONE)
    {

      digitalWrite(SF_DENDRO_EN_PIN, LOW);
      pinMode(I2C_SCL, INPUT);
      pinMode(I2C_SDA, INPUT);
      // Serial.println("Turning off Dendro and SF: ");

      if (!SDI12_CONNECTED)
      {
        rtc_gpio_hold_dis(GPIO_NUM_5);
        gpio_reset_pin(GPIO_NUM_5);
      }

      // Serial.println("This is test code");
    }

    if ((DENDRO_DONE && SF_DONE && !SDI12_DONE))
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
          break;
        case 2:
          if (testCS655())
          {
            SDI12_DONE = 1;
            Serial.println("CS655 Measurements Done");
          }
          break;
        }
      }
    }

    if (BATT_DONE && SM_DONE && ST_DONE && DENDRO_DONE && SF_DONE && SDI12_DONE && DHT22_DONE)
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
      wilo.bootCount = bootCount;
      if (wilo.i2cDeviceType == I2C_MUX)
      {
        writeToI2CMuxFiles();
      }
      writeToSD();

      if (SDI12_CONNECTED)
      {
        writeToSD_SDI12();
      }

#endif

      LoRaSetup();

#ifdef ENABLE_SD

#endif

      SDWRITE_DONE = 1;
    }

#ifdef ENABLE_LORA

    os_runloop_once();
    // delay(1);
#endif
  }
#endif
}
