
#include <OneWire.h>
#include <DallasTemperature.h>
#include "pinmapping.h"
#include "DataProcessing_Driver.h"
#include "MeasureFlags.h"
#include "debug.h"

void readDS18B20();

extern float DS18B20median;