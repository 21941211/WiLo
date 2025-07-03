#include<Arduino.h>
#include "LoRa_Driver.h"
#include "debug.h"



extern RTC_DATA_ATTR int bootCount;


void print_wakeup_reason();
void wakeUp(uint8_t measureComplete);
void goSleep(int sleepTime);