#include "SoilMoisture_Driver.h"

#define SM_SAMPLE_SIZE 10
#define SM_TRIM_SIZE 5

float arrSM[SM_SAMPLE_SIZE] = {0};
float slope = 2.48;      // slope from linear fit
float intercept = -0.72; // intercept from linear fit
float SM_Vol = 0;

int airValue = 8191;
int waterValue = 5016;


int sampleCounterSM = 0;
long lastMillisSM = 0;

void SM_Measure()
{

  long currentMillis = millis();
  if (currentMillis >= (lastMillisSM + 100))
  {
    arrSM[sampleCounterSM] = analogRead(SOIL_MOISTURE_PIN);
    lastMillisSM = millis();
    sampleCounterSM++;
  }
  else
  {
    //Serial.println("SM not ready!");
    return;
  }
  float voltage, vol_water_cont; // preallocate to approx. voltage and theta_v
  uint16_t adc;

  if (sampleCounterSM == SM_SAMPLE_SIZE)
  {
    bubbleSort(arrSM, SM_SAMPLE_SIZE);
    adc = trimmedMean(arrSM, SM_SAMPLE_SIZE, SM_TRIM_SIZE);

    Serial.println("ADC value: " +String(adc));
  int SM_Percentage = map(adc,airValue,waterValue,0,100);


    voltage = (adc * 2600.0) / 8191.0;

  if(SM_Percentage>=100){
    SM_Percentage=100;
  }
  if(SM_Percentage<=0){
    SM_Percentage=0;
  }

      Serial.println("Soil moisture percentage :" + String(SM_Percentage)+ "%");

    Serial.println("SM Done:");

    Serial.print("Measured Voltage: ");
    Serial.print(voltage); // read sensor
    Serial.println(" mV");

    //float correctedVoltageSM = correctVoltage(voltage); // correct voltage

    // Serial.print(" mV, Corrected Voltage: ");
    // Serial.print(correctedVoltageSM); // corrected voltage
    Serial.print(" mV, Theta_v: ");
    vol_water_cont = ((1.0 / (voltage / 1000.0)) * slope) + intercept; // calc of theta_v (vol. water content)
    SM_Vol = voltage;
    Serial.print(vol_water_cont);
    Serial.println(" cm^3/cm^3"); // cm^3/cm^3
    Serial.println("******************************************************");
    SM_DONE = 1;
    return;
  }
}

// void SM_Measure() {

//  float voltage,vol_water_cont; // preallocate to approx. voltage and theta_v

//   Serial.println("Moisture value: ");

// uint16_t adc;

// for (uint8_t i = 0; i < SM_SAMPLE_SIZE; i++)
// {
// arrSM[i] = analogRead(SOIL_MOISTURE_PIN);
// Serial.print("SM ADC: ");
// Serial.println(arrSM[i]);
// delay(100);
// }
// bubbleSort(arrSM, SM_SAMPLE_SIZE);
// adc = trimmedMean(arrSM, SM_SAMPLE_SIZE, SM_TRIM_SIZE);

// Serial.println(adc);

//  voltage = (adc*2600.0)/8191.0;

//  Serial.print("Measured Voltage: ");
//    Serial.print(voltage); // read sensor
//    Serial.println(" mV");

// float correctedVoltageSM = correctVoltage(voltage); // correct voltage

// Serial.print(" mV, Corrected Voltage: ");
//   Serial.print(correctedVoltageSM); // corrected voltage
//   Serial.print(" mV, Theta_v: ");
//   vol_water_cont = ((1.0/(correctedVoltageSM/1000.0))*slope)+intercept; // calc of theta_v (vol. water content)
//   SM_Vol = vol_water_cont;
//   Serial.print(vol_water_cont);
//   Serial.println(" cm^3/cm^3"); // cm^3/cm^3

// }
