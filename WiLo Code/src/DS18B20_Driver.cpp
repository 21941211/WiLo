#include "DS18B20_Driver.h"



#define DS18B20_SAMPLE_SIZE 5
#define DS18B20_TRIM_SIZE 3


int sampleCounterST = 0;
long lastMillisST = 0;

// GPIO where the DS18B20 is connected to
const int oneWireBus = DS1B20_PIN;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

float arrDS18B20[DS18B20_SAMPLE_SIZE] = {0};

float DS18B20median = 0;


// void readDS18B20() {
//   sensors.requestTemperatures(); 
// for (uint8_t i = 0; i < DS18B20_SAMPLE_SIZE; i++)
// {
//  arrDS18B20[i] = sensors.getTempCByIndex(0);
//   delay(750);
// }

// bubbleSort(arrDS18B20, DS18B20_SAMPLE_SIZE);
// DS18B20median = trimmedMean(arrDS18B20, DS18B20_SAMPLE_SIZE, DS18B20_TRIM_SIZE);

// Serial.println(DS18B20median);
//   //Serial.println(" C");

// }

void readDS18B20() {
 sensors.requestTemperatures();
  long currentMillis = millis();
  if (currentMillis >= (lastMillisST + 1000)){
arrDS18B20[sampleCounterST] = sensors.getTempCByIndex(0);
Serial.print("Soil temperature: ");
Serial.print(sampleCounterST);
Serial.print(": ");
Serial.println(arrDS18B20[sampleCounterST]);
 Serial.println("******************************************************");
lastMillisST = millis();
sampleCounterST++;
  }
else {
return;
}
if (sampleCounterST==DS18B20_SAMPLE_SIZE)
{
bubbleSort(arrDS18B20, DS18B20_SAMPLE_SIZE);
DS18B20median = trimmedMean(arrDS18B20, DS18B20_SAMPLE_SIZE, DS18B20_TRIM_SIZE);

Serial.println("Soil temperature Done:");

Serial.print(DS18B20median);
  Serial.println(" C");
  Serial.println("******************************************************");
  ST_DONE = 1;
  return;
}

}