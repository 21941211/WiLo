
#include "DHT22_Driver.h"




#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHT_SAMPLE_SIZE 10
#define DHT_TRIM_COUNT 5


//float tempMedian = 0;
//float humMedian = 0;

int sampleCounterDHT22 = 0;
long lastMillisDHT22 = 0;


float arrTemp[DHT_SAMPLE_SIZE] = {0};
float arrHum[DHT_SAMPLE_SIZE] = {0};

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

void DHTSetup() {
  dht.begin();
}

void DHT_Measure() {

  long currentMillis = millis();
  if (currentMillis >= (lastMillisDHT22 + 750)){
arrTemp[sampleCounterDHT22] = dht.readTemperature();
arrHum[sampleCounterDHT22] = dht.readHumidity();
Serial.println("Test DHT22:");
Serial.println(arrTemp[sampleCounterDHT22]);
Serial.println(arrHum[sampleCounterDHT22]);


lastMillisDHT22 = millis();
sampleCounterDHT22++;
  }
else {
  //Serial.println("DHT22 Not Ready!");
return;
}
if (sampleCounterDHT22==DHT_SAMPLE_SIZE)
{
bubbleSort(arrTemp, DHT_SAMPLE_SIZE);
bubbleSort(arrHum, DHT_SAMPLE_SIZE);

Serial.println("DHT22 Done:");

wilo.AT = trimmedMean(arrTemp, DHT_SAMPLE_SIZE, DHT_TRIM_COUNT);
wilo.RH = trimmedMean(arrHum, DHT_SAMPLE_SIZE, DHT_TRIM_COUNT);

if (isnan(wilo.AT) || isnan(wilo.RH )){
  Serial.println("DHT22 Error: NaN");
  wilo.AT = 9999.99;
  wilo.RH  = 9999.99;
  DHT22_DONE = 1;
  return;
}
else if (wilo.AT < -40 || wilo.AT > 80 || wilo.RH  < 0 || wilo.RH  > 100)
{
  Serial.println("DHT22 Error: Out of Range");
  wilo.AT = 1111.11;
  wilo.RH = 1111.11;
  DHT22_DONE = 1;
  return;
}
else
{

  Serial.println("DHT22 Measurement Successful");
  Serial.print(F("Temperature: "));
  Serial.print(wilo.AT);
  Serial.print(F(" C   Humidity: "));
  Serial.print(wilo.RH);
  Serial.println(F("%"));
}
  Serial.println("******************************************************");
  DHT22_DONE = 1;
  return;
}

}
