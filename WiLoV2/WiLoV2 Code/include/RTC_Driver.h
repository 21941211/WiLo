#include <Arduino.h>
#include <RTClib.h>
#include "config.h"


 uint8_t setupRTC();
void printRTCLoop() ;
bool isRTC_DS3231() ;


extern RTC_DS3231 rtc;