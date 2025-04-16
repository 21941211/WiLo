#include "Arduino.h"
#include "pinmapping.h"
#include "DataProcessing_Driver.h"
#include "ADC_Driver.h"
#include "MeasureFlags.h"
#include "debug.h"

void SM_Measure();
int averageSM();

extern float SM_Vol;