
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include "DataProcessing_Driver.h"
#include "MeasureFlags.h"
#include "debug.h"
#include "defaultWiLoData.h"

void readDS18B20();

//extern float DS18B20median;