#include "Arduino.h"
#include "pinmapping.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "ADS1110.h"
#include "ADS1110_platform.h"
#include "DataProcessing_Driver.h"
#include "Adafruit_MCP3421.h"
#include <Wire.h>
#include "MeasureFlags.h"
#include "debug.h"

extern float delta_x;
extern float xPos;
extern float microns;

void Dendro_Measure();
void dendroSetup();
int averageDendro();
void ADS1110_READ(void);



