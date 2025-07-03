#include "Arduino.h"
#include "config.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "DataProcessing_Driver.h"
#include "Adafruit_MCP3421.h"
#include <Wire.h>
#include "MeasureFlags.h"
#include "debug.h"
#include "I2C_Driver.h"
#include "defaultWiLoData.h"

extern float delta_x;
extern float xPos;
//extern float microns;

void Dendro_Measure();
void dendroSetup();
int averageDendro();
void ADS1110_READ(void);
void setDendroParameters();



